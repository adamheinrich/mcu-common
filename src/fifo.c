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
#include <mcu-common/fifo.h>
#include <mcu-common/critical.h>

bool fifo_init(struct fifo *fifo)
{
	assert(fifo != NULL);

	fifo->head = 0;
	fifo->tail = 0;
	fifo->full = false;
	fifo->empty = true;

	return true;
}

int fifo_read(struct fifo *fifo, void *dst, int count)
{
	assert(fifo != NULL);
	assert(dst != NULL);

	int n = 0;

	CRITICAL_ENTER();

	if (!fifo->empty) {
		char *dst_ptr = dst;
		volatile const char *src_ptr = fifo->buffer;
		fifo->full = false;

		for ( ; (n < count && !fifo->empty); n++) {
			size_t i = fifo->tail * fifo->element_size;
			for (size_t j = 0; j < fifo->element_size; j++)
				*(dst_ptr++) = src_ptr[i+j];

			fifo->tail++;
			if (fifo->tail >= fifo->capacity)
				fifo->tail = 0;

			if (fifo->tail == fifo->head)
				fifo->empty = true;
		}
	}

	CRITICAL_EXIT();

	return n;
}

int fifo_write(struct fifo *fifo, const void *src, int count)
{
	assert(fifo != NULL);
	assert(src != NULL);

	int n = 0;

	CRITICAL_ENTER();

	if (!fifo->full) {
		const char *src_ptr = src;
		volatile char *dst_ptr = fifo->buffer;
		fifo->empty = false;

		for ( ; (n < count && !fifo->full); n++) {
			size_t i = fifo->head * fifo->element_size;
			for (size_t j = 0; j < fifo->element_size; j++)
				dst_ptr[i+j] = *(src_ptr++);

			fifo->head++;
			if (fifo->head >= fifo->capacity)
				fifo->head = 0;

			if (fifo->head == fifo->tail)
				fifo->full = true;
		}
	}

	CRITICAL_EXIT();

	return n;
}
