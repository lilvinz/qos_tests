
# Set up a default goal
.DEFAULT_GOAL := all

# Common UT
include $(ROOT_DIR)/src/common/ut/library.mk
# ChibiOS with Quantec HAL
include $(ROOT_DIR)/submodules/qos/hal/platforms/Posix/library.mk
# Format
include $(ROOT_DIR)/submodules/format/library.mk
CFLAGS += -DFORMAT_INCLUDE_FLOAT

# Compiler flags
ifdef NDEBUG
    CFLAGS += -O2 -flto -ggdb -fomit-frame-pointer -falign-functions=16 -falign-loops=16
else
    CFLAGS += -O0 -ggdb
endif
CFLAGS += -Wall -Werror -Wshadow
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
# ChibiOS configuration for simulation
CFLAGS += -DSIMULATOR
CFLAGS += -DCH_DBG_ENABLE_STACK_CHECK=FALSE
CFLAGS += -Wno-attributes
CFLAGS += -Wno-redundant-decls
CFLAGS += -m32
CFLAGS += -D_GNU_SOURCE

CONLYFLAGS += -std=gnu99

LDFLAGS += -lrt

include $(ROOT_DIR)/make/unittest.mk

# Include the dependency files.
include $(wildcard $(OUTDIR)/*.d)
