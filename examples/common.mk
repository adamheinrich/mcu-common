# Project name
BIN = $(notdir $(CURDIR))

# Include paths and source files
MCU_COMMON_DIR = ../..
OPENCM3_DIR = ../lib/libopencm3

INC = -I$(OPENCM3_DIR)/include \
      -L$(OPENCM3_DIR)/lib \
      -I$(MCU_COMMON_DIR)/inc

SRC_C = $(wildcard $(MCU_COMMON_DIR)/src/*.c)

SRC_LD = $(BUILD_DIR)/$(OPENCM3_DEVICE).ld

# Setup the target MCU used with libopencm3
OPENCM3_DEVICE = stm32f415rgt
OPENCM3_TARGET = stm32/f4
DEF = -DSTM32F4

# CPU architecture
ARCHFLAGS = -mcpu=cortex-m4
FPFLAGS = -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Black Magic Probe setup (optional):
#BLACKMAGIC_PORT = /dev/ttyACM0

include ../lib/cm-makefile/config.mk
include ../lib/cm-makefile/blackmagic.mk
include ../lib/cm-makefile/libopencm3.mk    # Builds libopencm3
include ../lib/cm-makefile/libopencm3_ld.mk # Generates linker script
include ../lib/cm-makefile/rules.mk
