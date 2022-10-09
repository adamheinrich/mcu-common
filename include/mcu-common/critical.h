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

#ifndef MCU_COMMON_CRITICAL_H
#define MCU_COMMON_CRITICAL_H

/**
 * Target-specific critical section macros
 *
 * Only ARM Cortex-M (ARMv6-M, ARMv7-M, ARMv7E-M) is currently supported.
 * For other architectures, the CRITICAL_ENTER() and CRITICAL_EXIT() macros
 * expand to empty stubs.
 *
 * @defgroup critical_defs Critical section macros
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@{*/

#if defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_7M__) || \
    defined(__ARM_ARCH_7EM__)

/**
 * Enters critical section (ARM Cortex-M).
 * Must be followed by CRITICAL_EXIT().
 */
#define CRITICAL_ENTER() \
	do { \
		uint32_t _prm; \
		__asm__ volatile ("mrs %0, primask" : "=r" (_prm)); \
		__asm__ volatile ("cpsid i" ::: "memory")

/**
 * Exits critical section (ARM Cortex-M).
 * Must be preceded by CRITICAL_ENTER().
 */
#define CRITICAL_EXIT() \
		__asm__ volatile ("msr primask, %0" :: "r" (_prm) : "memory"); \
	} while(0)

#else

#pragma message "Using empty stubs for CRITICAL_*() macros. "\
		"Please define them for your architecture."

#define CRITICAL_ENTER()	do {
#define CRITICAL_EXIT()		} while(0)

#endif

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* MCU_COMMON_CRITICAL_H */
