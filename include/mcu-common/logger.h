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

/**
 * The maximum number of logger_put() arguments supported by the implementation
 * @ingroup logger_module
 */
#ifndef LOGGER_MAX_ARGC
#define LOGGER_MAX_ARGC 6
#endif

/** Logger entry (used internally) */
/* TODO: Packed, uint8_t for argc? */
struct logger_entry {
	/** Number of arguments in #argv (0 to #LOGGER_MAX_ARGC) */
	int argc;
	/** Format string to be passed to `sprintf` */
	const char *fmt;
	/** Array of arguments to be passed to `sprintf` */
	unsigned int argv[LOGGER_MAX_ARGC];
};

/** @addtogroup logger_module
 @{ */

/** Logger instance */
struct logger {
	/**
	  * Pointer to write callback implemented by driver.
	  *
	  * The callback is called from logger_process() during deferred
	  * processing. The callback provides the driver with a string which is
	  * supposed to be written to the output interface (e.g. serial port).
	  *
	  * @param[in] str      Pointer to the string to be written
	  *                     (formatted by `sprintf`), not null-terminated
	  * @param length       Count of characters to be written
	  */
	void (*write_cb)(const char *str, size_t length);
	/** Pointer to #fifo instance for storing #logger_entry entries */
	struct fifo *fifo;
	/** Pointer to a buffer used to store string composed by `sprintf`. */
	char *str;
	/** Size of the string buffer. */
	size_t str_size;
	/** Logger initialized flag (handled internally) */
	bool initialized;
};

/**
 * Initializes the #logger instance and allocates its string and
 * @ref fifo_module buffers.
 *
 * @param log           Pointer to the #logger structure
 * @param log_write_cb  Pointer to write callback implemented by driver
 *                      (see logger.write_cb for details)
 * @param log_capacity  Capacity of the internal @ref fifo_module (maximum
 *                      number of messages to be stored, see fifo.capacity)
 * @param str_capacity  Capacity of the internal string buffer (should be large
 *                      enough to store a message composed by `snprintf`, see
 *                      logger.str and logger.str_size)
 */
#define LOGGER_INIT(log, log_write_cb, log_capacity, str_capacity) \
	do { \
		static struct fifo logger_fifo; \
		FIFO_INIT(&logger_fifo, sizeof(struct logger_entry), \
			 (log_capacity)); \
		static char str[(str_capacity)]; \
		(log)->fifo = &logger_fifo; \
		(log)->write_cb = (log_write_cb); \
		(log)->str = (str); \
		(log)->str_size = (str_capacity); \
		(log)->write_cb = (log_write_cb); \
		logger_init((log)); \
	} while (0)

/**
 * Logs a message (shortcut for logger_put() which automatically determines
 * the number of arguments).
 *
 * @param log   Pointer to the #logger structure
 * @param ...   Format string and up to #LOGGER_MAX_ARGC optional arguments
 *              to be passed to `sprintf` (the format string is mandatory).
 */
#define LOGGER_PUT(log, ...) \
	logger_put((log), VA_ARGC(__VA_ARGS__)-1, __VA_ARGS__)

bool logger_init(struct logger *log);
bool logger_put(const struct logger *log, int argc, const char *fmt, ...);
bool logger_process(const struct logger *log);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* MCU_COMMON_LOGGER_H */
