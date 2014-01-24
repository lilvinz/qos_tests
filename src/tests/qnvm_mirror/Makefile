
# Set up a default goal
.DEFAULT_GOAL := all

# Common
include $(ROOT_DIR)/src/common/library.mk
# ChibiOS with Quantec HAL
include $(ROOT_DIR)/src/qhal/platforms/Posix/library.mk

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

CONLYFLAGS += -std=gnu99

LDFLAGS += -lrt

include $(ROOT_DIR)/make/unittest.mk

# Include the dependency files.
include $(wildcard $(OUTDIR)/*.d)
