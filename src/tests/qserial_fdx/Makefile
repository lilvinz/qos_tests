
# Set up a default goal
.DEFAULT_GOAL := all

# Common UT
include $(ROOT_DIR)/src/common/ut/library.mk
# QOS
include $(ROOT_DIR)/submodules/qos/hal/ports/simulator/posix/library.mk
include $(ROOT_DIR)/submodules/qos/common/ports/SIMIA32/compilers/GCC/library.mk
# Chibios
include $(ROOT_DIR)/submodules/chibios/os/hal/osal/rt/osal.mk
include $(ROOT_DIR)/submodules/chibios/os/rt/rt.mk
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
CFLAGS += -Wno-attributes
CFLAGS += -Wno-redundant-decls
CFLAGS += -m32
CFLAGS += -D_GNU_SOURCE

CONLYFLAGS += -std=gnu99

LDFLAGS += -lrt

include $(ROOT_DIR)/make/unittest.mk

# Include the dependency files.
include $(wildcard $(OUTDIR)/*.d)
