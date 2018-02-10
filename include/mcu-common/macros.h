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

#ifndef MCU_COMMON_MACROS_H
#define MCU_COMMON_MACROS_H

/**
 * Common macros
 * @defgroup macros_defs Common macros
 */

#define VA_ARGC_IMPL(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, \
		     a14, a15, a16, n, ...)	(n)

/**@{*/

/** Number of arguments for variadic macros (works for up to 16 arguments) */
#define VA_ARGC(...)	VA_ARGC_IMPL(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, \
			9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/** Size of an array */
#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(*(x)))

/**@}*/

#endif /* MCU_COMMON_MACROS_H */
