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

#include <stdio.h>
#include <stdlib.h>

#include "pico/malloc.h"
#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "libam2315.h"

int main(void);

int
main()
{

	stdio_init_all();

	struct env *config;

	config = (struct env *) malloc(sizeof(struct env *));

	/*config->i2c_addr = AM2315_ADDR;*/
	config->i2c_addr = AM2315C_ADDR;
	config->i2c_channel = I2C0;
	config->i2c_speed = 400000;
	config->started = false;

	AM2315 am(config);

	while(config->started == false);

	printf("\nReady to read!\n");
	free(config);

	while(1) {
		am.read_all(config);
		printf("%.2fc, %.2f%%\n", am.temperature, am.humidity);
		am.temperature *= 1.8;
		am.temperature += 32;
		printf("%.2ff, %.2f%%\n", am.temperature, am.humidity);
		sleep_ms(READ_WAIT);
	}

	return 0;
}
