
include $(ROOT_DIR)/make/firmware-defs.mk

CFLAGS += -O0
CFLAGS += -Wall -Werror -Wshadow
CFLAGS += -g
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

CONLYFLAGS += -std=gnu99

include $(ROOT_DIR)/make/unittest.mk
