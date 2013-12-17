
# Set up a default goal
.DEFAULT_GOAL := all

# Format
include $(ROOT_DIR)/src/lib/format/library_with_float.mk

CFLAGS += -O0
CFLAGS += -Wall -Werror -Wshadow
CFLAGS += -g
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

CONLYFLAGS += -std=gnu99

LDFLAGS += -lrt

include $(ROOT_DIR)/make/unittest.mk
