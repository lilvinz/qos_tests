
include ../common/common.mk
include $(ROOT_DIR)/make/firmware-defs.mk

# Target file name (without extension).
TARGET := $(BUILD_PREFIX)_$(BOARD_NAME)

# Common Bootloader
include $(ROOT_DIR)/src/common/bl/library.mk
# QOS
include $(ROOT_DIR)/submodules/qos/hal/ports/STM32/STM32F4xx/library.mk
include $(ROOT_DIR)/submodules/qos/common/ports/ARMCMx/compilers/GCC/library.mk
# Chibios
include $(ROOT_DIR)/submodules/chibios/os/hal/osal/rt/osal.mk
include $(ROOT_DIR)/submodules/chibios/os/rt/rt.mk
include $(ROOT_DIR)/submodules/chibios/os/rt/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk

# List modules to include in this build here
MODULES += $(notdir $(wildcard $(ROOT_DIR)/src/modules/*))

# Add files from enabled module directories
CSRC += $(foreach module, $(MODULES), $(wildcard $(ROOT_DIR)/src/modules/$(module)/*.c))
EXTRAINCDIRS += $(foreach module, $(MODULES), $(wildcard $(ROOT_DIR)/src/modules/$(module)))

# List C source files here
CSRC += $(wildcard ./*.c)
CSRC += $(filter-out $(addprefix %,$(notdir $(wildcard ./*.c))), $(wildcard ../common/*.c))

# List Assembler source files here
ASRC +=

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += .
EXTRAINCDIRS += ../common

# Place project-specific -D (define) and/or 
# -U options for C here.
ifdef NDEBUG
    CFLAGS += -DNDEBUG
    CFLAGS += -DCORTEX_USE_FPU=TRUE
    CFLAGS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
else
    CFLAGS += -DCORTEX_USE_FPU=FALSE
endif
CFLAGS += -DCORTEX_VTOR_INIT=$$(($(BL_ORIGIN) - $(EF_ORIGIN)))

# Place project-specific -D and/or -U options for 
# Assembler with preprocessor here.
ifdef NDEBUG
    ASFLAGS += -DNDEBUG
    ASFLAGS += -DCORTEX_USE_FPU=TRUE
    ASFLAGS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
else
    ASFLAGS += -DCORTEX_USE_FPU=FALSE
endif
ASFLAGS += -D__ASSEMBLY__

# Extra libraries
EXTRA_LIBS += c
EXTRA_LIBS += gcc

# Compiler flags
CFLAGS += -O2 -flto -specs=nano.specs -ggdb -fomit-frame-pointer -falign-functions=16 -falign-loops=16
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -Wimplicit -Wpointer-arith -Wredundant-decls -Wshadow -Wno-unused-parameter
CFLAGS += -Wno-unused-but-set-variable -Wunsuffixed-float-constants -Wstrict-prototypes
CFLAGS += -ffunction-sections -fdata-sections -fno-common
CFLAGS += -fno-builtin-printf -nodefaultlibs
CFLAGS += -MMD -MP -MF $(OUTDIR)/$(@F).d
CFLAGS += -std=gnu99
CFLAGS += -mcpu=cortex-m4
ifdef NDEBUG
    CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
endif
CFLAGS += $(patsubst %, -I%, $(EXTRAINCDIRS))

# Assembler flags
ASFLAGS += -mcpu=cortex-m4
ifdef NDEBUG
    ASFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
endif
ASFLAGS += $(patsubst %, -I%, $(EXTRAINCDIRS))
ASFLAGS += -x assembler-with-cpp

# Linker flags
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %, -l%, $(EXTRA_LIBS))
# this will intentionally lead to linker failure upon newlib usage of _malloc_r
LDFLAGS += -Wl,-wrap,_malloc_r
# tell linker to use thumb mode
LDFLAGS += -mthumb
# add memory section definitions in front of all linker flags
LDFLAGS := -Tmemory.ld $(LDFLAGS)

#============================================================================

# List of all source files.
ALLSRC = $(CSRC) $(ASRC)

# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(ALLSRC)))

# Define all object files.
ALLOBJ = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

# Define all depedency-files (used for make clean).
DEPFILES = $(addprefix $(OUTDIR)/dep/, $(addsuffix .o.d, $(ALLSRCBASE)))

# Default target.
.PHONY: all
all: elf bin lss sym

.PHONY: elf
elf: $(OUTDIR)/$(TARGET).elf

.PHONY: bin
bin: $(OUTDIR)/$(TARGET).bin

.PHONY: lss
lss: $(OUTDIR)/$(TARGET).lss

.PHONY: sym
sym: $(OUTDIR)/$(TARGET).sym

# Assemble: create object files from assembler source files.
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_THUMB_TEMPLATE, $(src))))

# Compile: create object files from C source files.
$(foreach src, $(CSRC), $(eval $(call COMPILE_C_THUMB_TEMPLATE, $(src))))

# Link: create ELF output file from object files.
$(eval $(call LINK_C_TEMPLATE, $(OUTDIR)/$(TARGET).elf, $(ALLOBJ)))

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE, $(OUTDIR)/$(TARGET).bin, $(BL_ORIGIN),$(BL_SIZE), $(OPENOCD_JTAG_CONFIG), $(OPENOCD_TRANSPORT), $(OPENOCD_CONFIG)))

# Include the dependency files.
-include $(wildcard $(OUTDIR)/*.d)
