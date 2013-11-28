# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------

include Makefile.inc
include Makefile_lint.inc


# Directory for build files (obj, dep etc.)
BUILDDIR = build

# Directory for output files (elf, sym, map, hex, bin etc.)
OUTDIR = firmware

# Target file name (without extension).
TARGET = main

TARGET_HW ?= TARGET_MULTI
BUILD_TYPE ?= RELEASE_BUILD

# List precompiled header files here.
PCHSRC = stdheader.h

# Paths to libraries
APPLIBDIR = ./Libraries
FREERTOSDIR = $(APPLIBDIR)/FreeRTOS
FREERTOSSRCDIR = $(APPLIBDIR)/FreeRTOS/Source
FREERTOSINCDIR = $(APPLIBDIR)/FreeRTOS/Source/include
STMLIBDIR = $(APPLIBDIR)
STMSPDDIR = $(STMLIBDIR)/STM32F2xx_StdPeriph_Driver
STMSPDSRCDIR = $(STMSPDDIR)/src
STMSPDINCDIR = $(STMSPDDIR)/inc
STMUSBDEVLIBDIR = $(STMLIBDIR)/STM32_USB_Device_Library
STMUSBCOREDIR = $(STMUSBDEVLIBDIR)/Core
STMUSBCORESRCDIR = $(STMUSBCOREDIR)/src
STMUSBCOREINCDIR = $(STMUSBCOREDIR)/inc
STMUSBCLASSCDCDIR = $(STMUSBDEVLIBDIR)/Class/cdc
STMUSBCLASSCDCSRCDIR = $(STMUSBCLASSCDCDIR)/src
STMUSBCLASSCDCINCDIR = $(STMUSBCLASSCDCDIR)/inc
STMUSBOTGDRIVERDIR = $(STMLIBDIR)/STM32_USB_OTG_Driver
STMUSBOTGDRIVERSRCDIR = $(STMUSBOTGDRIVERDIR)/src
STMUSBOTGDRIVERINCDIR = $(STMUSBOTGDRIVERDIR)/inc
CMSISDIR  = $(STMLIBDIR)/CMSIS
LIBDCCDIR = $(APPLIBDIR)/libdcc
LIBDEBUGDIR = $(APPLIBDIR)/Lib_DEBUG
LIBGETOPTDIR = $(APPLIBDIR)/freegetopt-0.11


TERMINALDIR = terminal
DRIVERDIR = driver
USBCOMMONDIR = usb_common
MISCDIR = misc
SYSTEMDIR = system
MAINDIR = main
NETWORKDIR = network

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

# Attention, image_information must no be compiled with pch!
MAINSRC += image_information.c
MAINSRC += bootloader.c
MAINSRC += main.c 
MAINSRC += stm32f2xx_it.c
MAINSRC += psu_monitoring.c
MAINSRC += sensors.c
MAINSRC += sequence.c
MAINSRC += aguard.c
MAINSRC += detector.c
MAINSRC += gloom_manager.c
MAINSRC += bmm.c
MAINSRC += bmm_profiles.c
MAINSRC += syndication.c
MAINSRC += firmware_store.c
MAINSRC += time_meter.c
MAINSRC += sv_uni_com.c
MAINSRC += svinfo_manager.c
MAINSRC += measureport.c
MAINSRC += version.c
MAINSRC += astroclock.c
CSRC += $(addprefix $(MAINDIR)/, $(MAINSRC))
EXTRALIBINCDIRS += $(MAINDIR)

TERMINALSRC += telnet.c
TERMINALSRC += terminal_server.c
TERMINALSRC += cli.c
TERMINALSRC += cli_commands.c
TERMINALSRC += cmd_factorytest.c
TERMINALSRC += cmd_calibrate.c
CSRC += $(addprefix $(TERMINALDIR)/, $(TERMINALSRC))
EXTRALIBINCDIRS += $(TERMINALDIR)

DRIVERSRC += hw_configuration.c
DRIVERSRC += interrupts.c
DRIVERSRC += hw_clock.c
DRIVERSRC += driver.c 
DRIVERSRC += dim.c 
DRIVERSRC += i2c.c 
DRIVERSRC += spi_master.c
DRIVERSRC += spi_serial_flash.c 
DRIVERSRC += spi_serial_fram.c 
DRIVERSRC += status_led.c 
DRIVERSRC += mems_lis302dl.c 
DRIVERSRC += gloom.c 
DRIVERSRC += lm75.c 
DRIVERSRC += adc.c 
DRIVERSRC += indication.c 
DRIVERSRC += uart.c 
DRIVERSRC += can_stm32_f2xx.c 
DRIVERSRC += can_cmd_i.c 
DRIVERSRC += visibility.c 
DRIVERSRC += visibility_vaisala_pwd20.c 
DRIVERSRC += gpssync.c
DRIVERSRC += sst_90_w.c
DRIVERSRC += sst_90_r.c
DRIVERSRC += led_compensation.c
DRIVERSRC += piezo.c
DRIVERSRC += svinfo.c
DRIVERSRC += fram_mirror.c
DRIVERSRC += voltcraft_lx_1108.c
DRIVERSRC += vcp_uart.c
DRIVERSRC += mesh.c 
DRIVERSRC += heater.c 
CSRC += $(addprefix $(DRIVERDIR)/, $(DRIVERSRC))
EXTRALIBINCDIRS += $(DRIVERDIR)

USBCOMMONSRC += usbd_desc.c
USBCOMMONSRC += usb_bsp.c
USBCOMMONSRC += usbd_cdc_vcp.c
USBCOMMONSRC += usbd_usr.c
CSRC += $(addprefix $(USBCOMMONDIR)/, $(USBCOMMONSRC))
EXTRALIBINCDIRS += $(USBCOMMONDIR)

SYSTEMSRC += memtest.c
SYSTEMSRC += syscalls.c
SYSTEMSRC += printf-stdarg.c 
SYSTEMSRC += clock.c 
SYSTEMSRC += console.c 
SYSTEMSRC += settings.c 
SYSTEMSRC += systemconfig.c 
SYSTEMSRC += log.c 
SYSTEMSRC += local_texts_log.c 
SYSTEMSRC += timer.c
SYSTEMSRC += system.c
SYSTEMSRC += init.c
SYSTEMSRC += sound.c
SYSTEMSRC += xmodem.c
SYSTEMSRC += fw_transfer.c
SYSTEMSRC += external_firmware.c
SYSTEMSRC += lfsr.c
SYSTEMSRC += watchdog.c
SYSTEMSRC += valmart.c
SYSTEMSRC += DBG_printf_stdarg.c
SYSTEMSRC += alarm_clock.c
SYSTEMSRC += statistics.c
SYSTEMSRC += coredump.c
SYSTEMSRC += primestruct.c
CSRC += $(addprefix $(SYSTEMDIR)/, $(SYSTEMSRC))
EXTRALIBINCDIRS += $(SYSTEMDIR)

NETWORKSRC += network.c 
NETWORKSRC += meshcom_io.c 
NETWORKSRC += meshcom_bc.c 
NETWORKSRC += meshcom_cmd.c 
NETWORKSRC += meshcom_conn.c 
NETWORKSRC += meshcom_firmware_tx.c 
NETWORKSRC += mesh_fw_manager.c 
NETWORKSRC += mesh_testmode.c 
NETWORKSRC += modbus_rtu.c
NETWORKSRC += modbus_master.c
NETWORKSRC += modbus_control.c
NETWORKSRC += can_manager.c
CSRC += $(addprefix $(NETWORKDIR)/, $(NETWORKSRC))
EXTRALIBINCDIRS += $(NETWORKDIR)

MISCSRC += state.c
MISCSRC += fifo.c  
MISCSRC += rtc.c 
MISCSRC += password.c 
MISCSRC += md5.c 
MISCSRC += debug.c
MISCSRC += random.c
MISCSRC += hash.c
CSRC += $(addprefix $(MISCDIR)/, $(MISCSRC))
EXTRALIBINCDIRS += $(MISCDIR)


# CMSIS for STM32
CLIBSRC += $(CMSISDIR)/core_cm3.c
CLIBSRC += $(CMSISDIR)/system_stm32f2xx.c
EXTRALIBINCDIRS += $(CMSISDIR)

# STM-Library
STMSPDSRC += stm32f2xx_adc.c
STMSPDSRC += stm32f2xx_can.c
STMSPDSRC += stm32f2xx_crc.c
STMSPDSRC += stm32f2xx_cryp.c
STMSPDSRC += stm32f2xx_cryp_aes.c
STMSPDSRC += stm32f2xx_cryp_des.c
STMSPDSRC += stm32f2xx_cryp_tdes.c
STMSPDSRC += stm32f2xx_dac.c
STMSPDSRC += stm32f2xx_dbgmcu.c
STMSPDSRC += stm32f2xx_dcmi.c
STMSPDSRC += stm32f2xx_dma.c
STMSPDSRC += stm32f2xx_exti.c
STMSPDSRC += stm32f2xx_flash.c
STMSPDSRC += stm32f2xx_fsmc.c
STMSPDSRC += stm32f2xx_gpio.c
STMSPDSRC += stm32f2xx_hash.c
STMSPDSRC += stm32f2xx_hash_md5.c
STMSPDSRC += stm32f2xx_hash_sha1.c
STMSPDSRC += stm32f2xx_i2c.c
STMSPDSRC += stm32f2xx_iwdg.c
STMSPDSRC += stm32f2xx_pwr.c
STMSPDSRC += stm32f2xx_rcc.c
STMSPDSRC += stm32f2xx_rng.c
STMSPDSRC += stm32f2xx_rtc.c
STMSPDSRC += stm32f2xx_sdio.c
STMSPDSRC += stm32f2xx_spi.c
STMSPDSRC += stm32f2xx_syscfg.c
STMSPDSRC += stm32f2xx_tim.c
STMSPDSRC += stm32f2xx_usart.c
STMSPDSRC += stm32f2xx_wwdg.c
STMSPDSRC += misc.c
CLIBSRC += $(addprefix $(STMSPDSRCDIR)/, $(STMSPDSRC))
EXTRALIBINCDIRS += $(STMSPDINCDIR)

# STM USB Library Core
STMUSBCORESRC += usbd_core.c
STMUSBCORESRC += usbd_ioreq.c
STMUSBCORESRC += usbd_req.c
CLIBSRC += $(addprefix $(STMUSBCORESRCDIR)/, $(STMUSBCORESRC))
EXTRALIBINCDIRS += $(STMUSBCOREINCDIR)

# STM USB Library Class CDC
STMUSBCLASSCDCSRC += usbd_cdc_core.c
CLIBSRC += $(addprefix $(STMUSBCLASSCDCSRCDIR)/, $(STMUSBCLASSCDCSRC))
EXTRALIBINCDIRS += $(STMUSBCLASSCDCINCDIR)

# STM USB OTG Driver
STMUSBOTGDRIVERSRC += usb_core.c
STMUSBOTGDRIVERSRC += usb_dcd.c
STMUSBOTGDRIVERSRC += usb_dcd_int.c
CLIBSRC += $(addprefix $(STMUSBOTGDRIVERSRCDIR)/, $(STMUSBOTGDRIVERSRC))
EXTRALIBINCDIRS += $(STMUSBOTGDRIVERINCDIR)

# FreeRTOS Kernel
FREERTOSSRC += list.c
FREERTOSSRC += portable/GCC/ARM_CM3_MPU/port.c
FREERTOSSRC += portable/MemMang/heap_1.c
FREERTOSSRC += queue.c
FREERTOSSRC += tasks.c
FREERTOSSRC += timers.c
CLIBSRC += $(addprefix $(FREERTOSSRCDIR)/, $(FREERTOSSRC))
EXTRALIBINCDIRS += $(FREERTOSINCDIR) $(FREERTOSSRCDIR)/portable/GCC/ARM_CM3_MPU

# libdcc
CLIBSRC += $(LIBDCCDIR)/dcc_stdio.c
EXTRALIBINCDIRS += $(LIBDCCDIR)

# libdebug
CLIBSRC += $(LIBDEBUGDIR)/stm32f2xx_ip_dbg.c
EXTRALIBINCDIRS += $(LIBDEBUGDIR)

# freegetopt
CLIBSRC += $(LIBGETOPTDIR)/freegetopt.c
EXTRALIBINCDIRS += $(LIBGETOPTDIR)


# List Assembler source files here.
ASRC = $(CMSISDIR)/startup_stm32f2xx.s


# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level.)
OPT = s -fomit-frame-pointer
OPT = 0


# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += $(EXTRALIBINCDIRS)


# Place project-specific -D (define) and/or 
# -U options for C here.
CDEFS += -DIMAGE_INFO_BASE=$(IMAGE_INFO_BASE)
CDEFS += -DIMAGE_INFO_STRUCT_OFFSET=$(IMAGE_INFO_STRUCT_OFFSET)UL
CDEFS += -DMESH_IMAGE_INFO_STRUCT_OFFSET=$(MESH_IMAGE_INFO_STRUCT_OFFSET)UL
CDEFS += -DUSE_STDPERIPH_DRIVER
CDEFS += -DUSE_FULL_ASSERT
CDEFS += -DLFSR_GENERATOR_POLY=$(LFSR_GENERATOR_POLY) 
CDEFS += -DLFSR_KEY=$(LFSR_KEY) 
#CDEFS += -D_REENT_ONLY
CDEFS += -D$(TARGET_HW)
CDEFS += -D$(BUILD_TYPE)
CDEFS += -DVCS_REVISION=$(VCS_REVISION)
CDEFS += -DVCS_PATH=$(VCS_PATH)

#---------------- vcs environment ----------------
VCS_REVISION := $(shell git rev-parse --short=10 HEAD)
ifeq ($(GIT_BRANCH),)
  # try to get remote tracking branch
  VCS_PATH := $(shell git rev-parse --symbolic-full-name --abbrev-ref @{u} 2> /dev/null)
  # if that failed, get local branch
  ifeq ($(VCS_PATH),@{u})
    VCS_PATH := $(shell git rev-parse --abbrev-ref HEAD)
  endif
else
  VCS_PATH := $(GIT_BRANCH)
endif
GIT_CHECK_DIRTY := $(shell git diff)
ifneq ($(GIT_CHECK_DIRTY),)
  VCS_PATH := $(join $(VCS_PATH),+)
endif


# Place project-specific -D and/or -U options for 
# Assembler with preprocessor here.
ADEFS += -D__ASSEMBLY__

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
# for newlib-lpc (file: libnewlibc-lpc.a):
#    EXTRA_LIBS = newlib-lpc
EXTRA_LIBS = m gcc c

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
LDFLAGS += -nostartfiles -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
# Set linker-script name
LDFLAGS +=-T$(LINKERSCRIPTPATH)/$(CHIP)_flash_mpu.ld
LDFLAGS += -Wl,--warn-common
LDFLAGS += -Wl,--fatal-warnings



# -------------------------PC-LINT-------------------------------------------

PCLINT = lint-nt

PCLINT_COMPILER_INCS_ROOT = C:/Program Files (x86)/CodeSourcery/Sourcery G++ Lite/arm-none-eabi/include/
PCLINT_COMPILER_INCS = . sys machine bits
PCLINT_COMPILER_INCS_ROOT_2 = C:/Program Files (x86)/CodeSourcery/Sourcery G++ Lite/lib/gcc/arm-none-eabi/4.4.1/include/
PCLINT_COMPILER_INCS_2 = .

PCLINT_INCS = $(patsubst %,-i'$(PCLINT_COMPILER_INCS_ROOT)%', $(PCLINT_COMPILER_INCS))
PCLINT_INCS += $(patsubst %,-i'$(PCLINT_COMPILER_INCS_ROOT_2)%', $(PCLINT_COMPILER_INCS_2))
PCLINT_INCS += $(patsubst %,-i'%', $(EXTRAINCDIRS))

PCLINT_EXCL = $(patsubst %,"+libdir('%')", $(EXTRALIBINCDIRS)) "+libdir('$(PCLINT_COMPILER_INCS_ROOT)')" "+libdir('$(PCLINT_COMPILER_INCS_ROOT_2)')"

PCLINT_DEFS = $(CDEFS) -D__have_long64=0
PCLINT_SRCS = $(CSRC)
PCLINT_OPTS = +v -b -i$(CURDIR)/pclint -i$(CURDIR)/pclint/lnt std.lnt

RUN_PCLINT = $(PCLINT) $(PCLINT_INCS) $(PCLINT_EXCL) $(PCLINT_DEFS) $(PCLINT_OPTS) $(PCLINT_SRCS)


# ---------------------------------------------------------------------------

# List of all source files
ALLSRC = $(CSRC) $(CLIBSRC) $(ASRC)

# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(ALLSRC)))

# Define all object files.
ALLOBJ = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

# Define all pch object files.
PCHOBJ = $(addprefix $(BUILDDIR)/, $(addsuffix _pch.h.gch, $(notdir $(basename $(PCHSRC)))))

# Default target.
all: gccversion build sizeafter

dist: gccversion build enc

lint:
	@echo curdir $(CURDIR)
	@echo 
	@echo Running PC-LINT ...
	$(RUN_PCLINT)

mainonly: $(OUTDIR)/$(TARGET)_mainonly.bin
enc: $(OUTDIR)/$(TARGET)_mainonly.enc
elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin

build: elf mainonly bin hex lss sym

sizeafter: build
	@echo $(MSG_SIZE_AFTER)
	$(SIZE) --format=Berkeley --common $(OUTDIR)/$(TARGET).elf

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
.SECONDARY: $(OUTDIR)/$(TARGET).elf
.PRECIOUS: $(ALLOBJ)
$(OUTDIR)/$(TARGET).elf: $(ALLOBJ)
	@echo $(MSG_LINKING) $@
	@$(CC) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)
	@echo ---- Remove ARM.exidx section: $@
	@$(OBJCOPY) --remove-section=.ARM.exidx $@
	@echo ---- Setting IMAGE_INFO_CHECKSUM and IMAGE_INFO_SIZE $@
	@$(OBJCOPY) --gap-fill=0xff -O binary $@ $@.bin
	@perl ./tools/patch_image_information.pl $@.bin $@ $(IMAGE_INFO_BASE) $(IMAGE_INFO_STRUCT_OFFSET)
	@$(RM) $@.bin


# Create final output file (.bin) from ELF output file.
# We have to gap-fill by 0xff to match our flash contents with OpenOCD .elf file loading
%.bin: %.elf
	@echo $(MSG_LOAD_FILE) $@
	@$(OBJCOPY) --gap-fill=0xff -O binary $< $@

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

# Create final output file (_mainonly.bin) from ?.
%_mainonly.bin: %.bin
	@echo $(MSG_LOAD_FILE) $@
	@$(DD) if=$< of=$@ bs=1024 skip=32 > /dev/null 2>&1 || exit 1
	
# Create final output file (_mainonly.bin) from ?.
%.enc: %.bin
	@echo ---- Encrypting file: $@
	@perl $(LFSR_TOOL) $(LFSR_GENERATOR_POLY) $(LFSR_KEY) < $< > $@ || exit 1


# Target: clean project.
clean:
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OUTDIR)/$(TARGET).elf
	$(REMOVE) $(OUTDIR)/$(TARGET).hex
	$(REMOVE) $(OUTDIR)/$(TARGET).bin
	$(REMOVE) $(OUTDIR)/$(TARGET)_mainonly.bin
	$(REMOVE) $(OUTDIR)/$(TARGET).lss
	$(REMOVE) $(OUTDIR)/$(TARGET).sym
	$(REMOVE) $(OUTDIR)/$(TARGET).map
	$(REMOVE) $(ALLOBJ)
	$(REMOVE) $(PCHOBJ)
	$(REMOVE) -r $(BUILDDIR)/dep

# Create language files
$(shell if [ system/local_texts.txt -nt system/local_texts.c ]; then perl tools/generateLocalTexts.pl system/local_texts.txt >/dev/null 2>/dev/null; fi;)
$(shell if [ system/local_texts_log.txt -nt system/local_texts_log.c ]; then perl tools/generateLocalTexts.pl system/local_texts_log.txt >/dev/null 2>/dev/null; fi;)

# Create object files directory
$(shell $(MKDIR) $(BUILDDIR) 2>/dev/null)
$(shell $(MKDIR) $(OUTDIR) 2>/dev/null)

# Include the dependency files.
-include $(shell $(MKDIR) $(BUILDDIR)/dep 2>/dev/null) $(wildcard $(BUILDDIR)/dep/*)

# Listing of phony targets.
.PHONY: all lint sizeafter gccversion build elf bsl hex lss bin sym clean copy_firmware

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

ifeq ($(BUILD_TYPE),DEBUG_BUILD)
    DEBUG_BUILD_STRING := _debug
else
    DEBUG_BUILD_STRING :=
endif

.PHONY: flashtool
flashtool: FLASHTOOL_OPTIONS := TOP=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))/ BUILD_DIR=firmware/ PROGRAMMER=openocd PRODUCT_NAME=sirius_r2_$(call lc,$(TARGET_HW))$(DEBUG_BUILD_STRING) PRODUCT_FILES="firmware/main.bin firmware/main_mainonly.enc flashtool/*"
flashtool: all
	$(MAKE) -f ../submodules/flashtool/Makefile $(FLASHTOOL_OPTIONS) clean
	$(MAKE) -f ../submodules/flashtool/Makefile $(FLASHTOOL_OPTIONS)
