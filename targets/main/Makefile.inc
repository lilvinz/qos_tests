

# MCU name, submodel and board
# - MCU used for compiler-option (-mcpu)
CHIP = STM32F2xx_1024k_128k

# LFSR encryption parameters
LFSR_GENERATOR_POLY = 0xBE4F7253
LFSR_KEY = 0x3FC01A44

# Position of checksum in final image
IMAGE_INFO_STRUCT_OFFSET = 0x00000184
IMAGE_INFO_BASE = 0x08008000
MESH_IMAGE_INFO_STRUCT_OFFSET = 0x4
BOOTLOADER_FW_FILE = bootloader/build/bootloader.elf


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = \
	-mcpu=cortex-m3 \
	-mthumb \
	-Wall \
	-Wextra \
	-Werror \
	-Wimplicit \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wshadow \
	-Wno-unused-parameter \
	-Wnested-externs \
	-Winvalid-pch \
	-std=gnu99 \
	-gdwarf-2 \
	-pipe\
	-Wunsuffixed-float-constants

# Compiler flags to generate dependency files:
CFLAGS += -MMD -MP -MF $(BUILDDIR)/dep/$(@F).d


# Place project-specific -D (define) and/or 
# -U options for C here.
CDEFS = -DSTM32F2XX
CDEFS += -DHSE_VALUE=25000000UL
CDEFS += -DUSE_STDPERIPH_DRIVER

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Define programs and commands.
# Toolchain prefix (i.e arm-elf- -> arm-elf-gcc.exe)
TCHAIN_PREFIX = arm-none-eabi-
CC = $(TCHAIN_PREFIX)gcc
AR = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE = $(TCHAIN_PREFIX)size
NM = $(TCHAIN_PREFIX)nm
MKDIR = mkdir -p
REMOVE = rm -f
SHELL = bash
COPY = cp
DD = dd
LFSR_TOOL = ./tools/lfsr.pl

# Define Messages
MSG_SIZE_AFTER = ---- Size after build:
MSG_LOAD_FILE = ---- Creating load file:
MSG_EXTENDED_LISTING = ---- Creating Extended Listing/Disassembly:
MSG_SYMBOL_TABLE = ---- Creating Symbol Table:
MSG_LINKING = ---- Linking :
MSG_COMPILING = ---- Compiling C :
MSG_COMPILING_PCH = ---- Generating precompiled header :
MSG_ASSEMBLING = ---- Assembling:
MSG_CLEANING = ---- Cleaning project:


