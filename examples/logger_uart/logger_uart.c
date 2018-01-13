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

#include "logger_uart.h"
#include <mcu-common/logger.h>
#include <mcu-common/critical.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

struct logger logger_uart;

static struct fifo tx_fifo;
static volatile bool tx_pending;

void usart2_isr(void)
{
	char c;

	if (((USART_CR1(USART2) & USART_CR1_TXEIE) != 0) &&
	    ((USART_SR(USART2) & USART_SR_TXE) != 0)) {

		if (fifo_read(&tx_fifo, &c, 1) == 1) {
			tx_pending = true;
			usart_send(USART2, c);
		} else {
			usart_disable_tx_interrupt(USART2);
			tx_pending = false;
		}
	}
}

static void uart_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART2);

	/* USART2_TX: */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

	/* USART2_RX: */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO3);

	usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX_RX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	/* TODO: Set NVIC_USART2_IRQ to the lowest priority */
	nvic_enable_irq(NVIC_USART2_IRQ);
	usart_enable(USART2);
}

static int uart_write(const char *str, size_t length)
{
	if (str == NULL || length == 0)
		return 0;

	CRITICAL_ENTER();

	if (tx_pending) {
		fifo_write(&tx_fifo, str, (int)length);
	} else {
		if (length > 1)
			fifo_write(&tx_fifo, &str[1], (int)length-1);
		tx_pending = true;
		usart_send(USART2, str[0]);
		usart_enable_tx_interrupt(USART2);
	}

	CRITICAL_EXIT();

	return (int)length;
}

void logger_uart_init(void)
{
	uart_init();
	FIFO_INIT(&tx_fifo, sizeof(char), 1024);
	LOGGER_INIT(&logger_uart, &uart_write, 64, false);
}
