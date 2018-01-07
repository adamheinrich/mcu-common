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

#define LOGGER_MAX_ARGC 6

struct logger_entry {
	int argc; /* TODO: Packed, uint8_t? */
	const char *fmt;
	unsigned int argv[LOGGER_MAX_ARGC];
};

struct logger {
	FILE *fp;
	ssize_t (*write_cb)(const char *str, size_t length);
	struct fifo *fifo;
	bool initialized;
};

#define LOGGER_INIT(log, log_write_cb, log_capacity, log_buffered)	do { \
		static struct fifo logger_fifo; \
		FIFO_INIT(&logger_fifo, sizeof(struct logger_entry), \
			 (log_capacity)); \
		(log)->fifo = &logger_fifo; \
		(log)->write_cb = (log_write_cb); \
		logger_init((log), (log_buffered)); \
	} while (0)

#define LOGGER_PUT(log, ...) \
	logger_put((log), VA_ARGC(__VA_ARGS__)-1, __VA_ARGS__)

bool logger_init(struct logger *log, bool buffered);
bool logger_put(const struct logger *log, int argc, const char *fmt, ...);
bool logger_process(const struct logger *log);

#ifdef __cplusplus
}
#endif

#endif /* MCU_COMMON_LOGGER_H */
