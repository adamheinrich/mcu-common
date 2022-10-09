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
 * @defgroup logger_module Logger
 * Universal logger module with deferred processing
 */

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <mcu-common/logger.h>
#include <mcu-common/critical.h>

/**@{*/

static int snprintl(char *s, size_t n, const struct logger_entry *e);

/**
 * Initializes logger.
 *
 * @param log           Pointer to the #logger structure
 *
 * @return `true` if initialization succeeds, `false` otherwise
 */
bool logger_init(struct logger *log)
{
	assert(log != NULL);
	assert(log->write_cb != NULL);

	log->initialized = false;

	if (!fifo_init(log->fifo))
		return true;

	log->initialized = true;

	return true;
}

/**
 * Logs a message (puts it into internal buffer for later processing).
 *
 * The message consists of a format string and variable number of arguments
 * which will be processed by `sprintf` in logger_process(). To determine the
 * number of arguments (`argc`) automatically, use macro LOGGER_PUT() instead.
 *
 * The access to internal #fifo is protected by a critical section so
 * logger_put() can be called from multiple threads or interrupt handlers.
 *
 * @param log           Pointer to the #logger structure
 * @param argc          Number of arguments (0 to #LOGGER_MAX_ARGC)
 * @param[in] fmt       Format string to be passed to `sprintf`
 * @param ...           Optional arguments to be passed to `sprintf`
 *                      (only `argc` parameters will be processed)
 *
 * @return `true` if initialization succeeds, `false` otherwise
 * (internal @ref fifo_module is full)
 */
bool logger_put(const struct logger *log, int argc, const char *fmt, ...)
{
	assert(log != NULL);
	assert(fmt != NULL);
	assert(argc >= 0 && argc <= LOGGER_MAX_ARGC);

	if (!log->initialized)
		return false;

	struct logger_entry entry;
	va_list args;

	entry.fmt = fmt;
	entry.argc = (argc > LOGGER_MAX_ARGC) ? LOGGER_MAX_ARGC : argc;

	va_start(args, fmt);
	for (int i = 0; i < entry.argc; i++)
		entry.argv[i] = va_arg(args, unsigned int);
	va_end(args);

	size_t n;

	CRITICAL_ENTER();
	n = fifo_write(log->fifo, &entry, 1);
	CRITICAL_EXIT();

	return (n == 1);
}

/**
 * Processes a single logged message from the buffer.
 *
 * This function calls logger.write_cb callback (implemented by the driver) to
 * write the message to an actual output interface. It is meant to defer log
 * processing from high priority contexts (such as interrupts) and should be
 * therefore called in a low-priority context (e.g. a main loop).
 *
 * It is assumed this function is called from multiple threads (the internal
 * #fifo is not protected by any locking mechanism).
 *
 * @param log Pointer to the #logger structure
 *
 * @return `true` if a message has been processed, `false` otherwise
 * (internal @ref fifo_module is empty)
 */
bool logger_process(const struct logger *log)
{
	assert(log != NULL);

	if (!log->initialized)
		return false;

	struct logger_entry entry;
	if (fifo_read(log->fifo, &entry, 1) == 1) {
		int n = snprintl(log->str, log->str_size, &entry);
		if (n > 0) {
			size_t len = (size_t)n;
			if (len > log->str_size)
				len = log->str_size;
			log->write_cb(log->str, len);
		}
		return true;
	}

	return false;
}

static int snprintl(char *s, size_t n, const struct logger_entry *e)
{
	assert(s != NULL);
	assert(e != NULL);

	const char *fm = e->fmt;
	const unsigned int *a = e->argv;

	switch (e->argc) {
	case 0:
		return snprintf(s, n, fm);
	case 1:
		return snprintf(s, n, fm, a[0]);
	case 2:
		return snprintf(s, n, fm, a[0], a[1]);
	case 3:
		return snprintf(s, n, fm, a[0], a[1], a[2]);
	case 4:
		return snprintf(s, n, fm, a[0], a[1], a[2], a[3]);
	case 5:
		return snprintf(s, n, fm, a[0], a[1], a[2], a[3], a[4]);
	case 6:
		return snprintf(s, n, fm, a[0], a[1], a[2], a[3], a[4], a[5]);
#if (LOGGER_MAX_ARGC > 6)
	#error "Please define more cases for the given value of LOGGER_MAX_ARGC"
#endif
	default:
		return -1;
	}
}

/**@}*/
