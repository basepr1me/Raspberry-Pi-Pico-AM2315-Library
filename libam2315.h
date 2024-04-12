/*
 * Copyright (c) 2022, 2024 Tracey Emery <tracey@openbsd.org>
 * Copyright (c) 2021 Limor Fried/Ladyada for Adafruit Industries, for typing
 *     most of the SI5351 registers and values
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

#ifndef _LIBAM2315_H
#define _LIBAM2315_H

struct env {
	uint8_t		 i2c_addr;
	uint8_t		 i2c_channel;
	uint32_t	 i2c_speed;
	i2c_inst_t	*i2c;
	bool		 started;
};

enum I2C_t {
	I2C0,
	I2C1
};

/* change to what you want */
#define PICO_I2C0_SDA_PIN	 16
#define PICO_I2C0_SCL_PIN	 17
#define PICO_I2C1_SDA_PIN	 2
#define PICO_I2C1_SCL_PIN	 3

/* AM2315 Address */
#define AM2315_ADDR		 0x5C

/*AM2315 Register */
#define AM2315_REGISTER_READ	 0x03

#define READ_WAIT		 5000

class
AM2315
{
	public:
		AM2315(struct env *);
		void read_all(struct env *);
		float temperature;
		float humidity;

	private:
		bool read_data(void);
};

#endif // _LIBAM2315_H
