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

#ifndef MCU_COMMON_LOGGER_H
#define MCU_COMMON_LOGGER_H

#include <stdbool.h>
#include <stdio.h>
#include <mcu-common/macros.h>
#include <mcu-common/fifo.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The maximum number of arguments supported by the implementation
 * @ingroup logger_module
 */
#define LOGGER_MAX_ARGC 6

/** @brief Logger entry (used internally)
 */
struct logger_entry {
	/** @brief Number of arguments in @ref argv
	 *
	 * Accepted values are 0 to @ref LOGGER_MAX_ARGC.
	 */
	int argc; /* TODO: Packed, uint8_t? */
	/** @brief Format string to be passed to `fprintf` */
	const char *fmt;
	/** @brief Array of arguments to be passed to `fprintf` */
	unsigned int argv[LOGGER_MAX_ARGC];
};

/** @brief Logger instance
 * @ingroup logger_module
 */
struct logger {
	/** @brief Pointer to a stream
	 * (assigned internally in @ref logger_init) */
	FILE *fp;
	/** @brief Write callback to be implemented by a driver
	  *
	  * The callback is called from @ref logger_process during deferred
	  * processing. The callback provides the driver with a string which is
	  * supposed to be written to the output interface (e.g. serial port).
	  *
	  * @param[in] str Pointer to the string to be written (formatted by
	  * `fprintf`), not null-terminated
	  * @param length Count of characters to be written
	  */
	ssize_t (*write_cb)(const char *str, size_t length);
	/** @brief Pointer to @ref fifo instance for storing @ref logger_entry
	 *  entries. */
	struct fifo *fifo;
	/** @brief Logger initialized flag (handled internally) */
	bool initialized;
};

/** @brief Allocate buffer for the internal @ref fifo_module and initialize
 * @ref logger instance
 * @ingroup logger_module
 */
#define LOGGER_INIT(log, log_write_cb, log_capacity, log_buffered)	do { \
		static struct fifo logger_fifo; \
		FIFO_INIT(&logger_fifo, sizeof(struct logger_entry), \
			 (log_capacity)); \
		(log)->fifo = &logger_fifo; \
		(log)->write_cb = (log_write_cb); \
		logger_init((log), (log_buffered)); \
	} while (0)

/** @brief Log a message (shortcut for @ref logger_put)
 *
 * This macro determines the number of arguments (`argc`) automatically.
 * @ingroup logger_module
 */
#define LOGGER_PUT(log, ...) \
	logger_put((log), VA_ARGC(__VA_ARGS__)-1, __VA_ARGS__)

bool logger_init(struct logger *log, bool buffered);
bool logger_put(const struct logger *log, int argc, const char *fmt, ...);
bool logger_process(const struct logger *log);

#ifdef __cplusplus
}
#endif

#endif /* MCU_COMMON_LOGGER_H */
