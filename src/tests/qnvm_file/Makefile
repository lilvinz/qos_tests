
# Set up a default goal
.DEFAULT_GOAL := all

# Common UT
include $(ROOT_DIR)/src/common/ut/library.mk
# ChibiOS with Quantec HAL
include $(ROOT_DIR)/submodules/qos/hal/platforms/Posix/library.mk

CFLAGS += -Os
CFLAGS += -Wall -Werror -Wshadow
CFLAGS += -g
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
# ChibiOS configuration for simulation
CFLAGS += -DSIMULATOR
CFLAGS += -DCH_DBG_ENABLE_STACK_CHECK=FALSE
CFLAGS += -Wno-attributes
CFLAGS += -Wno-redundant-decls
CFLAGS += -m32
CFLAGS += -D_GNU_SOURCE

# LFSR encryption parameters
LFSR_GENERATOR_POLY := 0xbe4f7253
LFSR_KEY := 0x3fc01a44

# Position of checksum in final image
IMAGE_INFO_STRUCT_OFFSET := 0x00000188
MESH_IMAGE_INFO_STRUCT_OFFSET := 0x4

CFLAGS += -DIMAGE_INFO_STRUCT_OFFSET=$(IMAGE_INFO_STRUCT_OFFSET)UL
CFLAGS += -DMESH_IMAGE_INFO_STRUCT_OFFSET=$(MESH_IMAGE_INFO_STRUCT_OFFSET)UL
CFLAGS += -DLFSR_GENERATOR_POLY=$(LFSR_GENERATOR_POLY)
CFLAGS += -DLFSR_KEY=$(LFSR_KEY)

CONLYFLAGS += -std=gnu99

LDFLAGS += -lrt

include $(ROOT_DIR)/make/unittest.mk

# Include the dependency files.
include $(wildcard $(OUTDIR)/*.d)
