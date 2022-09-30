/*
 * This file is part of MCU-Common.
 *
 * Copyright (C) 2017 Adam Heinrich <adam@adamh.cz>
 *
 * MCU-Common is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCU-Common is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MCU-Common.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the copyright holders of this library give
 * you permission to link this library with independent modules to
 * produce an executable, regardless of the license terms of these
 * independent modules, and to copy and distribute the resulting
 * executable under terms of your choice, provided that you also meet,
 * for each linked independent module, the terms and conditions of the
 * license of that module.  An independent module is a module which is
 * not derived from or based on this library.  If you modify this
 * library, you may extend this exception to your version of the
 * library, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include <assert.h>
#include <stdbool.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "logger_uart.h"

void __assert_func(const char *file, int line, const char *func,
		   const char *failedexpr)
{
	if (func == NULL)
		LOG("%s:%d: Assertion '%s' failed.\n", file, line, failedexpr);
	else
		LOG("%s:%d: %s(): Assertion '%s' failed.\n", file, line, func,
		    failedexpr);

	while(1) {
		logger_process(&logger_uart);
	}
}

void sys_tick_handler(void)
{
	static unsigned int i;

	gpio_toggle(GPIOA, GPIO8);
	LOG("%s(): i=%d\n", __func__, i);
	i++;
}

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	/* Turn the on-board LED on: */
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
	gpio_clear(GPIOA, GPIO8);

	systick_clear();
	systick_set_reload(rcc_ahb_frequency / 50);
	systick_counter_enable();
	systick_interrupt_enable();

	/* Initialize and test the logger module: */
	logger_uart_init();
	LOG("Hello World!\n");
	LOG("Buld date: %s (%s)\n", __DATE__, __TIME__);
	LOG("Six args: [ %d, %d, %d, %d, %d, %d ]\n", 10, 20, 30, 40, 50, 60);

	unsigned int loops = 0;
	unsigned int i = 0;

	while (1) {
		logger_process(&logger_uart);

		if (loops < 500000) {
			loops++;
		} else {
			loops = 0;
			LOG("%s(): i=%d\n", __func__, i);
			i++;
		}
	}
}
