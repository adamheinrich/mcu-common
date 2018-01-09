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

/* TODO: This implementation wastes one element, change it.
   It is currently only possible to write (capacity-1) elements. */

bool fifo_init(struct fifo *fifo)
{
	assert(fifo != NULL);

	fifo->head = 0;
	fifo->tail = 0;

	return true;
}

int fifo_read(struct fifo *fifo, void *dst, int count)
{
	assert(fifo != NULL);
	assert(dst != NULL);

	int num_read;
	char *dst_ptr = dst;
	volatile const char *src_ptr = fifo->buffer;

	CRITICAL_ENTER();

	for (num_read = 0; num_read < count; num_read++) {
		size_t next_tail = fifo->tail+1;
		if (next_tail >= fifo->capacity)
			next_tail = 0;

		if (fifo->tail == fifo->head) {
			break;
		} else {
			size_t i = fifo->tail * fifo->element_size;
			for (size_t j = 0; j < fifo->element_size; j++)
				*(dst_ptr++) = src_ptr[i+j];

			fifo->tail = next_tail;
		}
	}

	CRITICAL_EXIT();

	return num_read;
}

int fifo_write(struct fifo *fifo, const void *src, int count)
{
	assert(fifo != NULL);
	assert(src != NULL);

	int num_written;
	const char *src_ptr = src;
	volatile char *dst_ptr = fifo->buffer;

	CRITICAL_ENTER();

	for (num_written = 0; num_written < count; num_written++) {
		size_t next_head = fifo->head+1;
		if (next_head >= fifo->capacity)
			next_head = 0;

		if (next_head == fifo->tail) {
			break;
		} else {
			size_t i = fifo->head * fifo->element_size;
			for (size_t j = 0; j < fifo->element_size; j++)
				dst_ptr[i+j] = *(src_ptr++);

			fifo->head = next_head;
		}
	}

	CRITICAL_EXIT();

	return num_written;
}
