# Toolchain prefix (i.e arm-elf- -> arm-elf-gcc.exe)
TCHAIN_PREFIX ?= $(ARM_SDK_PREFIX)/arm-none-eabi-

# Define toolchain component names.
CC := $(TCHAIN_PREFIX)gcc
CPP := $(TCHAIN_PREFIX)g++
AR := $(TCHAIN_PREFIX)ar
OBJCOPY := $(TCHAIN_PREFIX)objcopy
OBJDUMP := $(TCHAIN_PREFIX)objdump
SIZE := $(TCHAIN_PREFIX)size
NM := $(TCHAIN_PREFIX)nm
STRIP := $(TCHAIN_PREFIX)strip
INSTALL := install
RM := rm

THUMB := -mthumb

# Test if quotes are needed for the echo-command
result = ${shell echo "test"}
ifeq (${result}, test)
    quote := '
# This line is just to clear out the single quote above '
else
     quote :=
endif

# Add a board designator to the terse message text
ifeq ($(ENABLE_MSG_EXTRA),yes)
    MSG_EXTRA := [$(BUILD_TYPE)|$(BOARD_NAME)]
else
    MSG_EXTRA :=
endif

# Define Messages
# English
MSG_FORMATERROR        = ${quote} Can not handle output-format${quote}
MSG_MODINIT            = ${quote} MODINIT   $(MSG_EXTRA) ${quote}
MSG_SIZE               = ${quote} SIZE      $(MSG_EXTRA) ${quote}
MSG_LOAD_FILE          = ${quote} BIN/HEX   $(MSG_EXTRA) ${quote}
MSG_BIN_OBJ            = ${quote} BINO      $(MSG_EXTRA) ${quote}
MSG_STRIP_FILE         = ${quote} STRIP     $(MSG_EXTRA) ${quote}
MSG_EXTENDED_LISTING   = ${quote} LIS       $(MSG_EXTRA) ${quote}
MSG_SYMBOL_TABLE       = ${quote} NM        $(MSG_EXTRA) ${quote}
MSG_LINKING_C          = ${quote} LDC       $(MSG_EXTRA) ${quote}
MSG_LINKING_CPP        = ${quote} LDCPP     $(MSG_EXTRA) ${quote}
MSG_COMPILING_THUMB    = ${quote} CC-THUMB  ${MSG_EXTRA} ${quote}
MSG_COMPILING          = ${quote} CC        ${MSG_EXTRA} ${quote}
MSG_COMPILINGCPP_THUMB = ${quote} CPP-THUMB $(MSG_EXTRA) ${quote}
MSG_COMPILINGCPP       = ${quote} CPP       $(MSG_EXTRA) ${quote}
MSG_ASSEMBLING_THUMB   = ${quote} AS-THUMB  $(MSG_EXTRA) ${quote}
MSG_ASSEMBLING         = ${quote} AS        $(MSG_EXTRA) ${quote}
MSG_CLEANING           = ${quote} CLEAN     $(MSG_EXTRA) ${quote}
MSG_ASMFROMC_THUMB     = ${quote} ASC-THUMB $(MSG_EXTRA) ${quote}
MSG_ASMFROMC           = ${quote} ASC       $(MSG_EXTRA) ${quote}
MSG_INSTALLING         = ${quote} INSTALL   $(MSG_EXTRA) ${quote}
MSG_FIRMWARE           = ${quote} FW        $(MSG_EXTRA) ${quote}
MSG_FWINFO             = ${quote} FWINFO    $(MSG_EXTRA) ${quote}
MSG_PADDING            = ${quote} PADDING   $(MSG_EXTRA) ${quote}
MSG_FLASH_IMG          = ${quote} FLASH_IMG $(MSG_EXTRA) ${quote}

toprel = $(subst $(realpath $(ROOT_DIR))/,,$(abspath $(1)))

# Display compiler version information.
.PHONY: gccversion
gccversion :
	@$(CC) --version

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O ihex $< $@

# Create stripped output file (.elf.stripped) from ELF output file.
%.elf.stripped: %.elf
	@echo $(MSG_STRIP_FILE) $(call toprel, $@)
	$(V1) $(STRIP) --strip-unneeded $< -o $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O binary $< $@

%.bin: %.o
	@echo $(MSG_LOAD_FILE) $(call toprel, $@)
	$(V1) $(OBJCOPY) -O binary $< $@

replace_special_chars = $(subst @,_,$(subst :,_,$(subst -,_,$(subst .,_,$(subst /,_,$1)))))
%.bin.o: %.bin
	@echo $(MSG_BIN_OBJ) $(call toprel, $@)
	$(V1) $(OBJCOPY) -I binary -O elf32-littlearm --binary-architecture arm \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		--wildcard \
		--redefine-sym _binary_$(call replace_special_chars,$<)_start=_binary_start \
		--redefine-sym _binary_$(call replace_special_chars,$<)_end=_binary_end \
		--redefine-sym _binary_$(call replace_special_chars,$<)_size=_binary_size \
		$< $@

# Create extended listing file/disassambly from ELF output file.
# using objdump testing: option -C
%.lss: %.elf
	@echo $(MSG_EXTENDED_LISTING) $(call toprel, $@)
	$(V1) $(OBJDUMP) -h -S -C -r $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo $(MSG_SYMBOL_TABLE) $(call toprel, $@)
	$(V1) $(NM) -n $< > $@

define SIZE_TEMPLATE
.PHONY: size
size: $(1)_size

.PHONY: $(1)_size
$(1)_size: $(1)
	@echo $(MSG_SIZE) $$(call toprel, $$<)
	$(V1) $(SIZE) -B $$<
endef

# firmware image template
#  $(1) = path to bin file
#  $(2) = boardtype in hex
#  $(3) = board revision in hex
define FW_TEMPLATE
FORCE:

$(1).firmwareinfo.c: $(1) $(ROOT_DIR)/make/templates/firmwareinfotemplate.c FORCE
	@echo $(MSG_FWINFO) $$(call toprel, $$@)
	$(V1) python $(ROOT_DIR)/make/scripts/version-info.py \
		--path=$(ROOT_DIR) \
		--template=$(ROOT_DIR)/make/templates/firmwareinfotemplate.c \
		--outfile=$$@ \
		--image=$(1) \
		--type=$(2) \
		--revision=$(3)

$(eval $(call COMPILE_C_THUMB_TEMPLATE, $(1).firmwareinfo.c))

$(OUTDIR)/$(notdir $(basename $(1))).fw : $(1) $(1).firmwareinfo.bin
	@echo $(MSG_FIRMWARE) $$(call toprel, $$@)
	$(V1) cat $(1) $(1).firmwareinfo.bin > $$@
endef

# Assemble: create object files from assembler source files.
define ASSEMBLE_THUMB_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_ASSEMBLING_THUMB) $$(call toprel, $$<)
	$(V1) $(CC) $(THUMB) -c $$(ASFLAGS) $$< -o $$@
endef

# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_ASSEMBLING) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(ASFLAGS) $$< -o $$@
endef

# Compile: create object files from C source files.
define COMPILE_C_THUMB_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILING_THUMB) $$(call toprel, $$<)
	$(V1) $(CC) $(THUMB) -c $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILING) $$(call toprel, $$<)
	$(V1) $(CC) -c $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create object files from C++ source files.
define COMPILE_CPP_THUMB_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILINGCPP_THUMB) $$(call toprel, $$<)
	$(V1) $(CPP) $(THUMB) -c $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@
endef

# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
	@echo $(MSG_COMPILINGCPP) $$(call toprel, $$<)
	$(V1) $(CPP) -c $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@
endef

# Compile: create assembler files from C source files.
define PARTIAL_COMPILE_THUMB_TEMPLATE
$($(1):.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC_THUMB) $$(call toprel, $$<)
	$(V1) $(CC) $(THUMB) -S $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Compile: create assembler files from C source files.
define PARTIAL_COMPILE_TEMPLATE
$($(1):.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $$(call toprel, $$<)
	$(V1) $(CC) -S $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@
endef

# Link: create ELF output file from object files.
#   $1 = elf file to produce
#   $2 = list of object files that make up the elf file
define LINK_C_TEMPLATE
.SECONDARY : $(1)
.PRECIOUS : $(2)
$(1):  $(2)
	@echo $(MSG_LINKING_C) $$(call toprel, $$@)
	$(V1) $(CC) $$(CFLAGS) $(2) --output $$@ $$(LDFLAGS)
endef

# Link: create ELF output file from object files.
#   $1 = elf file to produce
#   $2 = list of object files that make up the elf file
define LINK_CPP_TEMPLATE
.SECONDARY : $(1)
.PRECIOUS : $(2)
$(1):  $(2)
	@echo $(MSG_LINKING_CPP) $$(call toprel, $$@)
	$(V1) $(CPP) $$(CFLAGS) $(2) --output $$@ $$(LDFLAGS)
endef

# $(1) = Name of binary image to write
# $(2) = Base of flash region to write/wipe
# $(3) = Size of flash region to write/wipe
# $(4) = OpenOCD JTAG interface configuration file to use
# $(5) = OpenOCD configuration file to use
define JTAG_TEMPLATE
# ---------------------------------------------------------------------------
# Options for OpenOCD flash-programming
# see openocd.pdf/openocd.texi for further information

# if OpenOCD is in the $PATH just set OPENOCDEXE=openocd
OOCD_EXE ?= openocd

# debug level
OOCD_JTAG_SETUP  = -d0
# interface and board/target settings (using the OOCD target-library here)
OOCD_JTAG_SETUP += -s $(ROOT_DIR)/make/openocd
OOCD_JTAG_SETUP += -f $(4) -f $(5)

# initialize
OOCD_BOARD_RESET = -c "init"
# commands to prepare flash-write
OOCD_BOARD_RESET += -c "reset halt"

.PHONY: program
program: $(1)
	@echo $(MSG_JTAG_PROGRAM) $$(call toprel, $$<)
	$(V1) $(OOCD_EXE) \
		$$(OOCD_JTAG_SETUP) \
		$$(OOCD_BOARD_RESET) \
		-c "flash write_image erase $$< $(2) bin" \
		-c "verify_image $$< $(2) bin" \
		-c "reset run" \
		-c "shutdown"

.PHONY: wipe
wipe:
	@echo $(MSG_JTAG_WIPE) wiping $(3) bytes starting from $(2)
	$(V1) $(OOCD_EXE) \
		$$(OOCD_JTAG_SETUP) \
		$$(OOCD_BOARD_RESET) \
		-c "flash erase_address pad $(2) $(3)" \
		-c "reset run" \
		-c "shutdown"
endef

