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

/** @defgroup fifo_module FIFO
 *
 * @brief FIFO (first in, first out) queue implementation
 */

#include <assert.h>
#include <mcu-common/fifo.h>
#include <mcu-common/critical.h>

/**@{*/

/** @brief Initialize FIFO
 *
 * @param fifo Pointer to the @ref fifo structure
 *
 * @return `true` if initialization succeeds, `false` otherwise
 */
bool fifo_init(struct fifo *fifo)
{
	assert(fifo != NULL);

	fifo->head = 0;
	fifo->tail = 0;
	fifo->full = false;
	fifo->empty = true;

	return true;
}

/** @brief Get number of elements available to read from FIFO
 *
 * @param fifo Pointer to the @ref fifo structure
 *
 * @return The number of elements available for read (0 to `fifo.capacity`)
 */
int fifo_available(const struct fifo *fifo)
{
	assert(fifo != NULL);

	size_t n;

	CRITICAL_ENTER();

	if (fifo->empty) {
		n = 0;
	} else if (fifo->full) {
		n = fifo->capacity;
	} else if (fifo->head >= fifo->tail) {
		n = (fifo->head - fifo->tail);
	} else {
		n = (fifo->capacity - fifo->tail + fifo->head);
	}

	CRITICAL_EXIT();

	return (int)n;
}

/** @brief Read data from FIFO
 *
 * @param fifo Pointer to the @ref fifo structure
 * @param[out] dst Pointer where the read data will be stored to
 * @param count Number of elements to be read
 *
 * @return The number of elements actually read (0 to `count`)
 */
int fifo_read(struct fifo *fifo, void *dst, int count)
{
	assert(fifo != NULL);
	assert(dst != NULL);

	int n;
	char *ptr = dst;

	CRITICAL_ENTER();

	for (n = 0; (n < count && !fifo->empty); n++) {
		size_t i = fifo->tail * fifo->element_size;
		for (size_t j = 0; j < fifo->element_size; j++)
			*(ptr++) = ((char *)fifo->buffer)[i+j];

		fifo->tail++;
		if (fifo->tail >= fifo->capacity)
			fifo->tail = 0;

		if (fifo->tail == fifo->head)
			fifo->empty = true;
	}

	if (n > 0)
		fifo->full = false;

	CRITICAL_EXIT();

	return n;
}

/** @brief Write data to FIFO
 *
 * @param fifo Pointer to the @ref fifo structure
 * @param[in] src Pointer to the data written
 * @param count Number of elements to be written
 *
 * @return The number of elements actually written (0 to `count`)
 */
int fifo_write(struct fifo *fifo, const void *src, int count)
{
	assert(fifo != NULL);
	assert(src != NULL);

	int n;
	const char *ptr = src;

	CRITICAL_ENTER();

	for (n = 0; (n < count && !fifo->full); n++) {
		size_t i = fifo->head * fifo->element_size;
		for (size_t j = 0; j < fifo->element_size; j++)
			((char *)fifo->buffer)[i+j] = *(ptr++);

		fifo->head++;
		if (fifo->head >= fifo->capacity)
			fifo->head = 0;

		if (fifo->head == fifo->tail)
			fifo->full = true;
	}

	if (n > 0)
		fifo->empty = false;

	CRITICAL_EXIT();

	return n;
}

/** @brief Read null-terminated string from FIFO
 *
 * This function assumes `element_size` equals to one.
 *
 * @param fifo Pointer to the @ref fifo structure
 * @param[out] str Pointer where the string will be stored to
 *
 * @return Length of the string read (excluding terminating null-character)
 */
int fifo_gets(struct fifo *fifo, char *str)
{
	assert(fifo != NULL);
	assert(fifo->element_size == 1);
	assert(str != NULL);

	int n = 0;

	CRITICAL_ENTER();

	while (!fifo->full) {
		str[n] = ((char *)fifo->buffer)[fifo->tail];

		fifo->tail++;
		if (fifo->tail >= fifo->capacity)
			fifo->tail = 0;

		if (fifo->tail == fifo->head) {
			str[n+1] = '\0';
			fifo->empty = true;
		}

		if (str[n] == 0)
			break;

		n++;
	}

	if (n > 0)
		fifo->full = false;

	CRITICAL_EXIT();

	return n;
}

/** @brief Write null-terminated string to FIFO
 *
 * This function assumes `element_size` equals to one.
 *
 * @param fifo Pointer to the @ref fifo structure
 * @param[in] str Pointer to the string to be written
 *
 * @return Length of the string actually written (excluding terminating
 * null-character)
 */
int fifo_puts(struct fifo *fifo, const char *str)
{
	assert(fifo != NULL);
	assert(fifo->element_size == 1);
	assert(str != NULL);

	int n = 0;

	CRITICAL_ENTER();

	while (!fifo->full) {
		char *buffer = &((char *)fifo->buffer)[fifo->head];
		*buffer = str[n];

		fifo->head++;
		if (fifo->head >= fifo->capacity)
			fifo->head = 0;

		if (fifo->head == fifo->tail) {
			*buffer = '\0';
			fifo->full = true;
		}

		if (str[n] == 0)
			break;

		n++;
	}

	if (n > 0)
		fifo->empty = false;

	CRITICAL_EXIT();

	return n;
}


/**@}*/
