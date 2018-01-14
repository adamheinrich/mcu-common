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

#ifndef MCU_COMMON_FIFO_H
#define MCU_COMMON_FIFO_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Allocate buffer and initialize @ref fifo instance
 * @ingroup fifo_module
 */
#define FIFO_INIT(fifo, elem_size, fifo_capacity)	do { \
		static char buffer[(elem_size)*(fifo_capacity)]; \
		(fifo)->buffer = buffer; \
		(fifo)->element_size = (elem_size); \
		(fifo)->capacity = (fifo_capacity); \
		fifo_init(fifo); \
	} while (0)


/** @brief FIFO instance
 * @ingroup fifo_module
 */
struct fifo {
	/** @brief Pointer to the buffer used for FIFO
	 *
	 * Size of the buffer must be (@ref element_size * @ref capacity).
	 */
	volatile void *buffer;
	/** @brief Size of a single element */
	size_t element_size;
	/** @brief Number of elements in FIFO */
	size_t capacity;
	/** @brief Read index (handled internally) */
	volatile size_t tail;
	/** @brief Write index (handled internally) */
	volatile size_t head;
	/** @brief FIFO full flag (handled internally) */
	volatile bool full;
	/** @brief FIFO empty flag (handled internally) */
	volatile bool empty;
};

bool fifo_init(struct fifo *fifo);
int fifo_read(struct fifo *fifo, void *dst, int count);
int fifo_write(struct fifo *fifo, const void *src, int count);

#ifdef __cplusplus
}
#endif

#endif /* MCU_COMMON_FIFO_H */
