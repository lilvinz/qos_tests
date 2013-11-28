# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------

include ../Makefile.inc

# Directory for output files (obj, dep, elf, sym, map, hex, bin etc.)
BUILDDIR = build

# Target file name (without extension).
TARGET = bootloader

# List precompiled header files here.
PCHSRC = stdheader.h

# Paths to libraries
APPLIBDIR = ../Libraries
STMLIBDIR = $(APPLIBDIR)
STMSPDDIR = $(STMLIBDIR)/STM32F2xx_StdPeriph_Driver
STMSPDSRCDIR = $(STMSPDDIR)/src
STMSPDINCDIR = $(STMSPDDIR)/inc
STMUSBDIR = $(STMLIBDIR)/STM32_USB-FS-Device_Driver
STMUSBSRCDIR = $(STMUSBDIR)/src
STMUSBINCDIR = $(STMUSBDIR)/inc
CMSISDIR  = $(STMLIBDIR)/CMSIS
LIBDCCDIR = $(APPLIBDIR)/libdcc
LIBDEBUGDIR = $(APPLIBDIR)/Lib_DEBUG


# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
CSRC = bootloader.c
CSRC += bootloader_information.c
CSRC += printf-stdarg.c
CSRC += stm32f2xx_it.c
CSRC += delay.c
CSRC += uart.c
CSRC += spi_master.c
CSRC += spi_serial_flash.c
CSRC += spi_serial_fram.c
CSRC += fw_download.c
CSRC += fw_manager.c
CSRC += lfsr.c
CSRC += xmodem.c
CSRC += flashfromlocal.c
CSRC += external_firmware.c
CSRC += fram_mirror.c
#CSRC += usb_desc.c
#CSRC += usb_endp.c
#CSRC += usb_istr.c
#CSRC += usb_prop.c
#CSRC += usb_pwr.c

# CMSIS for STM32
CLIBSRC += $(CMSISDIR)/core_cm3.c
CLIBSRC += $(CMSISDIR)/system_stm32f2xx.c
EXTRALIBINCDIRS += $(CMSISDIR)

# STM-Library
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_adc.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_can.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_crc.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_cryp.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_cryp_aes.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_cryp_des.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_cryp_tdes.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_dac.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_dbgmcu.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_dcmi.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_dma.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_exti.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_flash.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_fsmc.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_gpio.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_hash.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_hash_md5.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_hash_sha1.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_i2c.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_iwdg.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_pwr.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_rcc.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_rng.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_rtc.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_sdio.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_spi.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_syscfg.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_tim.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_usart.c
CLIBSRC += $(STMSPDSRCDIR)/stm32f2xx_wwdg.c
CLIBSRC += $(STMSPDSRCDIR)/misc.c
EXTRALIBINCDIRS += $(STMSPDINCDIR)

# STM USB Library
#CLIBSRC += $(STMUSBSRCDIR)/usb_core.c
#CLIBSRC += $(STMUSBSRCDIR)/usb_init.c
#CLIBSRC += $(STMUSBSRCDIR)/usb_int.c
#CLIBSRC += $(STMUSBSRCDIR)/usb_mem.c
#CLIBSRC += $(STMUSBSRCDIR)/usb_regs.c
#CLIBSRC += $(STMUSBSRCDIR)/usb_sil.c
#EXTRALIBINCDIRS += $(STMUSBINCDIR)

# libdcc
CLIBSRC += $(LIBDCCDIR)/dcc_stdio.c
EXTRALIBINCDIRS += $(LIBDCCDIR)

# libdebug
CLIBSRC += $(LIBDEBUGDIR)/stm32f2xx_ip_dbg.c
EXTRALIBINCDIRS += $(LIBDEBUGDIR)

# List Assembler source files here.
# Make them always end in a capital .S. Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = $(CMSISDIR)/startup_stm32f2xx.s

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level.)
OPT = s -fomit-frame-pointer
#OPT = 0


# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += $(EXTRALIBINCDIRS)


# Place project-specific -D (define) and/or 
# -U options for C here.
CDEFS += -DUSE_FULL_ASSERT
CDEFS += -DIMAGE_INFO_BASE=$(IMAGE_INFO_BASE)UL
CDEFS += -DIMAGE_INFO_STRUCT_OFFSET=$(IMAGE_INFO_STRUCT_OFFSET)UL
CDEFS += -DIMAGE_INFO_STRUCT_OFFSET=$(IMAGE_INFO_STRUCT_OFFSET)UL
CDEFS += -DZIGBEE_IMAGE_INFO_STRUCT_OFFSET=$(ZIGBEE_IMAGE_INFO_STRUCT_OFFSET)UL
CDEFS += -DLFSR_GENERATOR_POLY=$(LFSR_GENERATOR_POLY) 
CDEFS += -DLFSR_KEY=$(LFSR_KEY) 

# Place project-specific -D and/or -U options for 
# Assembler with preprocessor here.
ADEFS += -D__ASSEMBLY__

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
# for newlib-lpc (file: libnewlibc-lpc.a):
#    EXTRA_LIBS = newlib-lpc
EXTRA_LIBS =

# Path to Linker-Scripts
LINKERSCRIPTPATH = .
LINKERSCRIPTINC = .


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS += -O$(OPT)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-builtin-printf
CFLAGS += -nodefaultlibs
CFLAGS += $(patsubst %.h,-include $(OBJDIR)/%_pch.h,$(HPPSRC))

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS += -mcpu=cortex-m3 -mthumb -I.
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-g$(DEBUG)
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))


# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS += $(LINKERSCRIPTINC)

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS += -nostartfiles -Wl,-Map=$(BUILDDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += -lc -lgcc 
# Set linker-script name
LDFLAGS +=-T$(LINKERSCRIPTPATH)/$(CHIP)_flash.ld


#============================================================================

# List of all source files.
ALLSRC = $(CSRC) $(CLIBSRC) $(ASRC)

# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(ALLSRC)))

# Define all object files.
ALLOBJ = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

# Define all pch object files.
PCHOBJ = $(addprefix $(BUILDDIR)/, $(addsuffix _pch.h.gch, $(notdir $(basename $(PCHSRC)))))

# Define all depedency-files (used for make clean).
DEPFILES = $(addprefix $(BUILDDIR)/dep/, $(addsuffix .o.d, $(ALLSRCBASE)))

# Default target.
all: gccversion build sizeafter

dist: all

elf: $(BUILDDIR)/$(TARGET).elf
lss: $(BUILDDIR)/$(TARGET).lss
sym: $(BUILDDIR)/$(TARGET).sym
hex: $(BUILDDIR)/$(TARGET).hex
bin: $(BUILDDIR)/$(TARGET).bin
header: $(BUILDDIR)/$(TARGET).hdr

build: elf bin hex lss sym header

sizeafter: build
	@echo $(MSG_SIZE_AFTER)
	@$(SIZE) --format=Berkeley --common $(BUILDDIR)/$(TARGET).elf

# Display compiler version information.
gccversion: 
	@$(CC) --version

# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(BUILDDIR)/$(notdir $(basename $(1))).o: $(1)
	@echo $(MSG_ASSEMBLING) $$<
	@$(CC) -c $$(ASFLAGS) $$< -o $$@
endef
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src))))

# Compile: create object files from C header files.
define COMPILE_H_TEMPLATE
$(BUILDDIR)/$(notdir $(basename $(1)))_pch.h.gch: $(1)
	@echo $(MSG_COMPILING) $$<
	@$(CC) -c $$(CFLAGS) $$< -o $$@ 
endef
$(foreach src, $(PCHSRC), $(eval $(call COMPILE_H_TEMPLATE, $(src))))

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(BUILDDIR)/$(notdir $(basename $(1))).o: $(1) $(PCHOBJ)
	@echo $(MSG_COMPILING) $$<
	@$(CC) -c $$(CFLAGS) $(patsubst %.h,-include $(BUILDDIR)/%_pch.h,$(PCHSRC)) $$< -o $$@ 
endef
$(foreach src, $(CSRC), $(eval $(call COMPILE_C_TEMPLATE, $(src))))

# Compile: create object files from library C source files.
define COMPILE_C_LIB_TEMPLATE
$(BUILDDIR)/$(notdir $(basename $(1))).o: $(1)
	@echo $(MSG_COMPILING) $$<
	@$(CC) -c $$(CFLAGS) $$< -o $$@ 
endef
$(foreach src, $(CLIBSRC), $(eval $(call COMPILE_C_LIB_TEMPLATE, $(src))))

# Link: create ELF output file from object files.
.SECONDARY: $(BUILDDIR)/$(TARGET).elf
.PRECIOUS: $(ALLOBJ)
$(BUILDDIR)/$(TARGET).elf: $(ALLOBJ)
	@echo $(MSG_LINKING) $@
	@$(CC) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)

# Create extended listing file/disassambly from ELF output file.
# using objdump testing: option -C
%.lss: %.elf
	@echo $(MSG_EXTENDED_LISTING) $@
	@$(OBJDUMP) -h -S -C -r $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo $(MSG_SYMBOL_TABLE) $@
	@$(NM) -n $< > $@
	@if [ ! "`grep malloc $@`" == "" ]; then \
		echo "using malloc is not allowed!"; \
		$(REMOVE) $@; \
		exit -1; \
	fi

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo $(MSG_LOAD_FILE) $@
	@$(OBJCOPY) -O ihex $< $@

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo $(MSG_LOAD_FILE) $@
	@$(OBJCOPY) --pad-to=$(IMAGE_INFO_BASE) -O binary $< $@

# Create c header file with firmware array
%.hdr: %.bin
	@echo ---- Embedding bootloader into header file $@
	@perl ../tools/bin2c.pl "unsigned char __attribute__ ((section (\".bootloader\"))) bootloader_firmware[] =" < $< > $@

# Target: clean project.
clean:
	@echo $(MSG_CLEANING)
	$(REMOVE) $(BUILDDIR)/$(TARGET).map
	$(REMOVE) $(BUILDDIR)/$(TARGET).elf
	$(REMOVE) $(BUILDDIR)/$(TARGET).hex
	$(REMOVE) $(BUILDDIR)/$(TARGET).bin
	$(REMOVE) $(BUILDDIR)/$(TARGET).sym
	$(REMOVE) $(BUILDDIR)/$(TARGET).lss
	$(REMOVE) $(BUILDDIR)/$(TARGET).hdr
	$(REMOVE) $(ALLOBJ)
	$(REMOVE) $(PCHOBJ)
	$(REMOVE) -r $(BUILDDIR)/dep

# Create object files directory
$(shell $(MKDIR) $(BUILDDIR) 2>/dev/null)

# Include the dependency files.
-include $(shell $(MKDIR) $(BUILDDIR)/dep 2>/dev/null) $(wildcard $(BUILDDIR)/dep/*)

# Listing of phony targets.
.PHONY : all sizeafter gccversion build elf hex bin lss sym bin clean

