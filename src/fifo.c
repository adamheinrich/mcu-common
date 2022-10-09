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

/**
 * @defgroup fifo_module FIFO
 *
 * FIFO (first in, first out) queue implementation
 *
 * The queue is lock-free as long as there is only one consumer and producer
 * thread (i.e. there cannot be multiple threads writing to the queue or
 * multiple threads reading from it). Use an appropriate locking mechanism
 * if there are multiple producers or consumers accessing the queue.
 *
 * The lock-free behavior is achieved by having a head index only updated
 * by the producer and a tail index only updated by the consumer, both in an
 * "atomic" way where it does not contain invalid intermediary values.
 * To distinguish between the empty `(head == tail)` and full `(head+1 == tail)`
 * states, a single element in the internal buffer is sacrificed as a trade-off
 * for not having a third "full" flag (which would have to be updated by both
 * consumer and producer, requiring a locking mechanism).
 *
 * The implementation is thus not lock-free on architectures where loading or
 * storing a `size_t` variable (used for the head and tail indexes) takes more
 * than a single instruction (e.g. 8-bit CPUs).
 */

#include <assert.h>
#include <mcu-common/fifo.h>

/**@{*/

/**
 * Initializes FIFO.
 *
 * @param fifo Pointer to the #fifo structure
 *
 * @return `true` if initialization succeeds, `false` otherwise
 */
bool fifo_init(struct fifo *fifo)
{
	assert(fifo != NULL);
	assert(fifo->buffer != NULL);
	assert(fifo->element_size > 0);
	assert(fifo->buffer_capacity > 1);

	fifo->head = 0;
	fifo->tail = 0;

	return true;
}

/**
 * Returns number of elements available to read from FIFO.
 *
 * @param fifo Pointer to the #fifo structure
 *
 * @return The number of elements available to read (0 to #fifo_capacity())
 */
size_t fifo_available(const struct fifo *fifo)
{
	assert(fifo != NULL);

	size_t head = fifo->head;
	size_t tail = fifo->tail;

	if (head == tail)
		return 0;
	else if (head > tail)
		return head - tail;
	else
		return fifo->buffer_capacity - tail + head;
}

/**
 * Returns number of elements the FIFO can hold.
 *
 * @param fifo Pointer to the #fifo structure
 *
 * @return The maximum number of elements the FIFO can hold
 */
size_t fifo_capacity(const struct fifo *fifo)
{
	assert(fifo != NULL);

	return fifo->buffer_capacity-1;
}

/**
 * Reads data from FIFO.
 *
 * @param fifo          Pointer to the #fifo structure
 * @param[out] dst      Pointer where the read data will be stored to
 * @param count         Number of elements to be read
 *
 * @return The number of elements actually read (0 to `count`)
 */
size_t fifo_read(struct fifo *fifo, void *dst, size_t count)
{
	assert(fifo != NULL);
	assert(dst != NULL);

	size_t n = 0;
	char *ptr = dst;
	size_t tail = fifo->tail;

	while (n < count) {
		if (tail == fifo->head) /* Fifo empty */
			break;

		size_t i = tail * fifo->element_size;
		for (size_t j = 0; j < fifo->element_size; j++)
			*(ptr++) = ((char *)fifo->buffer)[i+j];

		if (++tail == fifo->buffer_capacity)
			tail = 0;

		n++;
	}

	if (n)
		fifo->tail = tail;

	return n;
}

/**
 * Writes data to FIFO.
 *
 * @param fifo          Pointer to the #fifo structure
 * @param[in] src       Pointer to the data written
 * @param count         Number of elements to be written
 *
 * @return The number of elements actually written (0 to `count`)
 */
size_t fifo_write(struct fifo *fifo, const void *src, size_t count)
{
	assert(fifo != NULL);
	assert(src != NULL);

	size_t n = 0;
	const char *ptr = src;
	size_t head = fifo->head;

	while (n < count) {
		size_t next_head = head + 1;
		if (next_head == fifo->buffer_capacity)
			next_head = 0;

		if (next_head == fifo->tail) /* Fifo full */
			break;

		size_t i = head * fifo->element_size;
		for (size_t j = 0; j < fifo->element_size; j++)
			((char *)fifo->buffer)[i+j] = *(ptr++);

		head = next_head;
		n++;
	}

	if (n)
		fifo->head = head;

	return n;
}

/**
 * Reads null-terminated string from FIFO. This function assumes that
 * fifo.element_size equals to one.
 *
 * @param fifo          Pointer to the #fifo structure
 * @param[out] str      Pointer where the string will be stored to
 *
 * @return Length of the string read (excluding terminating null-character)
 */
size_t fifo_gets(struct fifo *fifo, char *str)
{
	assert(fifo != NULL);
	assert(str != NULL);

	size_t n = 0;
	size_t tail = fifo->tail;

	while (tail != fifo->head) {
		str[n] = ((char *)fifo->buffer)[tail];

		if (++tail == fifo->buffer_capacity)
			tail = 0;

		if (!str[n])
			break;

		n++;
	}

	str[n] = '\0';
	fifo->tail = tail;

	return n;
}

/**
 * Writes null-terminated string to FIFO. This function assumes that
 * fifo.element_size equals to one.
 *
 * @param fifo          Pointer to the #fifo structure
 * @param[in] str       Pointer to the string to be written
 *
 * @return Length of the string actually written (excluding terminating
 * null-character)
 */
size_t fifo_puts(struct fifo *fifo, const char *str)
{
	assert(fifo != NULL);
	assert(str != NULL);

	size_t n = 0;
	char *lastptr = NULL;
	size_t head = fifo->head;

	while (true) {
		size_t next_head = head + 1;
		if (next_head == fifo->buffer_capacity)
			next_head = 0;

		if (next_head == fifo->tail) { /* Fifo full */
			if (lastptr) {
				*lastptr = '\0';
				n--;
			}
			break;
		}

		lastptr = &((char *)fifo->buffer)[head];
		*lastptr = str[n];
		head = next_head;

		if (!str[n])
			break;

		n++;
	}

	fifo->head = head;

	return n;
}

/**@}*/
