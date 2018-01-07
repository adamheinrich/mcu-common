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
#include <stddef.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "uart.h"
#include "test_fifo.h"

static bool run_tests(void)
{
	return test_fifo();
}

void __assert_func(const char *file, int line, const char *func,
		   const char *failedexpr)
{
	if (func == NULL)
		uart_printf("%s:%d: Assertion '%s' failed.\n", file, line,
			    failedexpr);
	else
		uart_printf("%s:%d: %s(): Assertion '%s' failed.\n", file, line,
			    func, failedexpr);

	while(1);
}

int main(void)
{
	rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	/* Turn LED on: */
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
	gpio_clear(GPIOA, GPIO8);

	uart_init();
	uart_printf("mcu-common: tests\n");
	uart_printf("Build date: %s (%s)\n", __DATE__, __TIME__);

	run_tests();
	uart_printf("Done.\n");

	while (1);
}
