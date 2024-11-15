/*
 * Copyright (c) 2024 Tracey Emery <tracey@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "libam2315.h"

#define DEBUG		 1
#define DPRINTF(x,y)	 do { if (DEBUG) { printf("%s", x); sleep_ms(y); }} \
			    while(0)

static void		 begin(struct env *);
static void		 write(struct env *, const uint8_t *, size_t, bool);
static void		 read(struct env *, uint8_t *, size_t, bool);

AM2315::AM2315(struct env *config)
{
	if (DEBUG)
		DPRINTF("Initializing I2C\n", 500);

	if (config->i2c_addr == AM2315C_ADDR)
		config->c_sensor = true;
	else
		config->c_sensor = false;

	switch(config->i2c_channel) {
	case I2C1:
		config->i2c = i2c1;
		i2c_init(config->i2c, config->i2c_speed);
		gpio_set_function(PICO_I2C1_SDA_PIN, GPIO_FUNC_I2C);
		gpio_set_function(PICO_I2C1_SCL_PIN, GPIO_FUNC_I2C);
		gpio_pull_up(PICO_I2C1_SDA_PIN);
		gpio_pull_up(PICO_I2C1_SCL_PIN);
		break;
	case I2C0:
	default:
		config->i2c = i2c0;
		i2c_init(config->i2c, config->i2c_speed);
		gpio_set_function(PICO_I2C0_SDA_PIN, GPIO_FUNC_I2C);
		gpio_set_function(PICO_I2C0_SCL_PIN, GPIO_FUNC_I2C);
		gpio_pull_up(PICO_I2C0_SDA_PIN);
		gpio_pull_up(PICO_I2C0_SCL_PIN);
		break;
	}

	begin(config);
}

void
AM2315::read_all(struct env *config)
{
	uint8_t data[8];
	bool ready = false;
	uint32_t humidity_data = 0, temp_data = 0;

	memset(&data, 0, sizeof(data));
    	write(config, &data[0], 1, false);
	sleep_ms(50);

	if (config->i2c_addr == AM2315C_ADDR) {
		data[0] = AM2315C_REGISTER_READ;
		data[1] = 0x33;
		data[2] = 0x00;
	} else {
		data[0] = AM2315_REGISTER_READ;
		data[1] = 0x00;
	}	data[2] = 0x04;

	write(config, data, 3, false);

	if (config->i2c_addr == AM2315C_ADDR) {
		sleep_us(80);

		if (DEBUG)
			DPRINTF("Read AM2315C\n", 0);

		while (ready == false) {
			read(config, data, 1, true);
			if (data[0] & AM2315C_REGISTER_BUSY)
				sleep_us(100);
			else
				ready = true;
		}
		read(config, data, 6, true);
		humidity_data |= (data[1] << 16);
		humidity_data |= (data[2] << 8);
		humidity_data |= data[3];
		humidity_data = humidity_data >> 4;

		humidity = ((float)humidity_data * 100) / 0x100000;

		temp_data = (data[3] << 16);
		temp_data |= (data[4] << 8);
		temp_data |= data[5];
		temp_data = temp_data & ~(0xFFF00000);

		/* clear all data bits > 20 */
		temperature = ((float)temp_data * 200 / 0x100000) - 50;
	} else {
		if (DEBUG)
			DPRINTF("Read AM2315\n", 0);
		read(config, data, 8, true);

		if (data[0] != AM2315_REGISTER_READ) {
			if(DEBUG)
				DPRINTF("Didn't Read\n", 1000);
		}
		if (data[1] != 4) {
			if (DEBUG)
				DPRINTF("Short Read\n", 1000);
		}

		humidity = data[2];
		humidity *= 256;
		humidity += data[3];
		humidity /= 10;

		temperature = data[4] & 0x7F;
		temperature *= 256;
		temperature += data[5];
		temperature /= 10;

		if (data[4] >> 7)
			temperature = -temperature;
	}
}

static void
write(struct env *config, const uint8_t *src, size_t len, bool nostop)
{
	int ret;

	ret = i2c_write_blocking(config->i2c, config->i2c_addr, src, len,
	    nostop);
	sleep_us(200);
	if (ret == PICO_ERROR_GENERIC) {
		if (DEBUG)
			DPRINTF("WRITE FAILED!\n", 0);
	}
}

static void
read(struct env *config, uint8_t *dst, size_t len, bool nostop)
{
	int ret;

	ret = i2c_read_blocking(config->i2c, config->i2c_addr, dst, len,
	    nostop);
	sleep_us(200);
	if (ret < 0) {
		if (DEBUG)
			DPRINTF("READ FAILED!\n", 0);
	}
}

static void
begin(struct env *config)
{
	/* give device some init time */
	sleep_ms(150);
	config->started = true;

	/* init AM2315C devices */
	if (config->c_sensor) {
		uint8_t data[8];
		bool ready = false;

		data[0] = AM2315C_REGISTER_INIT;
		data[1] = 0x08;
		data[2] = 0x00;

		memset(&data, 0, sizeof(data));
	    	write(config, &data[0], 1, false);
		sleep_ms(50);
		while (ready == false) {
			read(config, data, 1, true);
			if (data[0] & AM2315C_REGISTER_BUSY)
				sleep_us(100);
			else
				ready = true;
		}

		read(config, data, 1, true);
		if (!(data[0] & AM2315C_REGISTER_CALD)) {
			if (DEBUG)
				DPRINTF("CONFIGURATION FAILED!\n", 0);
			while(1);
		}
	}
}
