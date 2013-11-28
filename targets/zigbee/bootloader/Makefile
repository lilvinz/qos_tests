#-------------------------------------------------------------------------------
#		User-modifiable options
#-------------------------------------------------------------------------------

# LFSR encryption parameters
LFSR_GENERATOR_POLY = 0xBE4F7253
LFSR_KEY = 0x3FC01A44

# Chip & board used for compilation
# (can be overriden by adding CHIP=chip and BOARD=board to the command-line)
CHIP  = at91sam7x256
BOARD = at91sam7x-ek

# Trace level used for compilation
# (can be overriden by adding TRACE_LEVEL=#number to the command-line)
# TRACE_LEVEL_DEBUG      5
# TRACE_LEVEL_INFO       4
# TRACE_LEVEL_WARNING    3
# TRACE_LEVEL_ERROR      2
# TRACE_LEVEL_FATAL      1
# TRACE_LEVEL_NO_TRACE   0
TRACE_LEVEL = 4

# Optimization level, put in comment for debugging
OPTIMIZATION = -Os

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# AT91 library directory
AT91LIB = ./at91lib

# Output file basename
OUTPUT = bootloader

# Compile for all memories available on the board (this sets $(MEMORIES))
#include $(AT91LIB)/boards/$(BOARD)/board.mak
MEMORIES = flash

# Output directories
BIN = bin
OBJ = obj

#-------------------------------------------------------------------------------
#		Tools
#-------------------------------------------------------------------------------

# Tool suffix when cross-compiling
CROSS_COMPILE = arm-none-eabi-

# Compilation tools
CC = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# Flags
INCLUDES = -I$(AT91LIB)/boards/$(BOARD) -I$(AT91LIB)/peripherals 
INCLUDES += -I$(AT91LIB)
INCLUDES += -I$(AT91LIB)/memories

CDEFS += -DLFSR_GENERATOR_POLY=$(LFSR_GENERATOR_POLY) 
CDEFS +=  -DLFSR_KEY=$(LFSR_KEY) 

CFLAGS = -Wall -mlong-calls -ffunction-sections
CFLAGS += -g $(OPTIMIZATION) $(INCLUDES) -D$(CHIP) -DTRACE_LEVEL=$(TRACE_LEVEL)
CFLAGS += $(CDEFS)
CFLAGS += $(CSTANDARD)
ASFLAGS = -g $(OPTIMIZATION) $(INCLUDES) -D$(CHIP) -D__ASSEMBLY__
LDFLAGS = -g $(OPTIMIZATION) -nostartfiles -Wl,--gc-sections



#-------------------------------------------------------------------------------
#		Files
#-------------------------------------------------------------------------------

# Directories where source files can be found
PERIPH = $(AT91LIB)/peripherals
BOARDS = $(AT91LIB)/boards
UTILITY = $(AT91LIB)/utility
MEM = $(AT91LIB)/memories

VPATH += $(MEM)/flash
VPATH += $(UTILITY)
VPATH += $(PERIPH)/dbgu $(PERIPH)/pio $(PERIPH)/usart $(PERIPH)/eefc $(PERIPH)/efc $(PERIPH)/pmc
VPATH += $(PERIPH)/cp15
VPATH += $(BOARDS)/$(BOARD) $(BOARDS)/$(BOARD)/$(CHIP)

# Objects built from C source files
C_OBJECTS = bootloader.o lfsr.o
C_OBJECTS += flashd_efc.o flashd_eefc.o
C_OBJECTS += stdio.o math.o usart.o
C_OBJECTS += dbgu.o pio.o efc.o eefc.o pmc.o cp15.o
C_OBJECTS += board_memories.o board_lowlevel.o

# Objects built from Assembly source files
ASM_OBJECTS = board_cstartup.o
ASM_OBJECTS += cp15_asm.o

# Append OBJ and BIN directories to output filename
OUTPUT := $(BIN)/$(OUTPUT)

#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------

all: gccversion $(BIN) $(OBJ) $(MEMORIES) copy

	# Display compiler version information.
gccversion:
	@$(CC) --version

$(BIN) $(OBJ):
	mkdir $@
	
copy:
	cp $(OUTPUT)-flash.elf bootloader.elf
	cp $(OUTPUT)-flash.bin bootloader.bin
	$(OBJDUMP) -h -S -C -r bootloader.elf > bootloader.lss


define RULES
C_OBJECTS_$(1) = $(addprefix $(OBJ)/$(1)_, $(C_OBJECTS))
ASM_OBJECTS_$(1) = $(addprefix $(OBJ)/$(1)_, $(ASM_OBJECTS))

$(1): $$(ASM_OBJECTS_$(1)) $$(C_OBJECTS_$(1))
	$(CC) $(LDFLAGS) -T"$(AT91LIB)/boards/$(BOARD)/$(CHIP)/$$@.lds" -o $(OUTPUT)-$$@.elf $$^
	$(OBJCOPY) -O binary $(OUTPUT)-$$@.elf $(OUTPUT)-$$@.bin
	$(OBJCOPY) -I binary -O elf32-littlearm --binary-architecture arm \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		--wildcard \
		--redefine-sym _binary_$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$(OUTPUT)-$$@.bin))))_start=_binary_start \
		--redefine-sym _binary_$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$(OUTPUT)-$$@.bin))))_end=_binary_end \
		--redefine-sym _binary_$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$(OUTPUT)-$$@.bin))))_size=_binary_size \
	$(OUTPUT)-$$@.bin $(OUTPUT)-$$@.bin.o
	$(SIZE) $$^ $(OUTPUT)-$$@.elf

$$(C_OBJECTS_$(1)): $(OBJ)/$(1)_%.o: %.c Makefile $(OBJ) $(BIN)
	$(CC) $(CFLAGS) -D$(1) -c -o $$@ $$<

$$(ASM_OBJECTS_$(1)): $(OBJ)/$(1)_%.o: %.S Makefile $(OBJ) $(BIN)
	$(CC) $(ASFLAGS) -D$(1) -c -o $$@ $$<

debug_$(1): $(1)
	perl ../resources/gdb/debug.pl $(OUTPUT)-$(1).elf

endef

$(foreach MEMORY, $(MEMORIES), $(eval $(call RULES,$(MEMORY))))

clean:
	-rm -f $(OBJ)/*.o $(BIN)/*.bin $(BIN)/*.elf

