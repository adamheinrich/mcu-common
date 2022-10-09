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

#include "test_fifo.h"
#include "test.h"
#include <stdint.h>
#include <string.h>
#include <mcu-common/fifo.h>
#include <mcu-common/macros.h>

static bool test_fifo_char(void)
{
	struct fifo fifo;
	FIFO_INIT(&fifo, sizeof(char), 10);

	for (char i = 0; i < (char)fifo_capacity(&fifo); i++) {
		TEST_ASSERT(fifo_write(&fifo, &i, 1) == 1);
	}

	char val = 42;
	TEST_ASSERT(fifo_write(&fifo, &val, 1) == 0);

	for (char i = 0; i < (char)fifo_capacity(&fifo); i++) {
		TEST_ASSERT(fifo_read(&fifo, &val, 1) == 1);
		TEST_ASSERT(val == i);
	}

	TEST_ASSERT(fifo_read(&fifo, &val, 1) == 0);

	return true;
}

static bool test_fifo_uint64(void)
{
	uint64_t val;
	struct fifo fifo;
	FIFO_INIT(&fifo, sizeof(uint64_t), 48);

	for (size_t i = 0; i < fifo_capacity(&fifo); i++) {
		val = (1UL << i);
		TEST_ASSERT(fifo_write(&fifo, &val, 1) == 1);
	}

	val = 0;
	TEST_ASSERT(fifo_write(&fifo, &val, 1) == 0);

	for (size_t i = 0; i < fifo_capacity(&fifo); i++) {
		TEST_ASSERT(fifo_read(&fifo, &val, 1) == 1);
		TEST_ASSERT(val == (1UL << i));
	}

	TEST_ASSERT(fifo_read(&fifo, &val, 1) == 0);

	return true;
}

static bool test_fifo_operations(void)
{
	static const int in[5] = { 1, 2, 3, 4, 5 };
	static int out[5];
	struct fifo fifo;

	FIFO_INIT(&fifo, sizeof(int), 5);

	TEST_ASSERT(fifo_available(&fifo) == 0);
	TEST_ASSERT(fifo_write(&fifo, in, 5) == 5); /* fifo: { 1, 2, 3, 4, 5 }*/

	TEST_ASSERT(fifo_available(&fifo) == 5);
	TEST_ASSERT(fifo_read(&fifo, out, 3) == 3); /* fifo: { 4, 5 } */
	TEST_ASSERT(out[0] == 1);
	TEST_ASSERT(out[1] == 2);
	TEST_ASSERT(out[2] == 3);

	TEST_ASSERT(fifo_available(&fifo) == 2);
	TEST_ASSERT(fifo_write(&fifo, in, 5) == 3); /* fifo: { 4, 5, 1, 2, 3 }*/

	TEST_ASSERT(fifo_available(&fifo) == 5);
	TEST_ASSERT(fifo_read(&fifo, out, 5) == 5); /* fifo: { } */
	TEST_ASSERT(out[0] == 4);
	TEST_ASSERT(out[1] == 5);
	TEST_ASSERT(out[2] == 1);
	TEST_ASSERT(out[3] == 2);
	TEST_ASSERT(out[4] == 3);

	TEST_ASSERT(fifo_available(&fifo) == 0);
	TEST_ASSERT(fifo_read(&fifo, out, 5) == 0);

	return true;
}

static bool test_fifo_str(void)
{
	struct fifo fifo;
	FIFO_INIT(&fifo, sizeof(char), 46);

	static const char *lines[] = {
		"A spectre is haunting Europe",	/* 29+1 */
		" -- ",				/* 4+1 */
		"the spectre of communism.",	/* 25+1 */
	};


	TEST_ASSERT(fifo_available(&fifo) == 0);

	TEST_ASSERT(fifo_puts(&fifo, lines[0]) == strlen(lines[0]));
	TEST_ASSERT(fifo_puts(&fifo, lines[1]) == strlen(lines[1]));

	size_t len12 = strlen(lines[0]) + strlen(lines[1]) + 2;
	TEST_ASSERT(fifo_available(&fifo) == len12);

	TEST_ASSERT(fifo_puts(&fifo, lines[2]) == 11); /* "the spectre" */

	static char str[32];
	for (size_t i = 0; i < 2; i++) {
		TEST_ASSERT(fifo_gets(&fifo, str) == strlen(str));
		TEST_ASSERT(strcmp(str, lines[i]) == 0);
	}

	TEST_ASSERT(fifo_available(&fifo) == 12); /* "the spectre" + '\0' */

	TEST_ASSERT(fifo_puts(&fifo, lines[0]) == strlen(lines[0]));
	TEST_ASSERT(fifo_puts(&fifo, lines[1]) == strlen(lines[1]));
	TEST_ASSERT(fifo_available(&fifo) == fifo_capacity(&fifo));

	int idx[] = { 2, 0, 1 };
	for (size_t i = 0; i < 3; i++) {
		TEST_ASSERT(fifo_gets(&fifo, str) == strlen(str));

		if (i == 0)
			TEST_ASSERT(strcmp(str, "the spectre") == 0);
		else
			TEST_ASSERT(strcmp(str, lines[idx[i]]) == 0);
	}

	TEST_ASSERT(fifo_available(&fifo) == 0);

	return true;
}

bool test_fifo(void)
{
	bool status = true;

	status &= TEST_RUN(test_fifo_char);
	status &= TEST_RUN(test_fifo_uint64);
	status &= TEST_RUN(test_fifo_operations);
	status &= TEST_RUN(test_fifo_str);

	return status;
}
