
# Set up a default goal
.DEFAULT_GOAL := all

# Common UT
include $(ROOT_DIR)/src/common/ut/library.mk
# Chibios
include $(ROOT_DIR)/submodules/chibios/os/hal/osal/rt/osal.mk
include $(ROOT_DIR)/submodules/chibios/os/rt/rt.mk
include $(ROOT_DIR)/submodules/chibios/os/rt/ports/SIMIA32/compilers/GCC/port.mk
# QOS
include $(ROOT_DIR)/submodules/qos/hal/ports/simulator/posix/library.mk
include $(ROOT_DIR)/submodules/qos/common/ports/SIMIA32/compilers/GCC/library.mk

CFLAGS += -O2 -flto -ggdb
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
