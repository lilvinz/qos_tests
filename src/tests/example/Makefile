
# Set up a default goal
.DEFAULT_GOAL := all

$(OUTDIR):
	$(V1) [ -d $@ ] || mkdir -p $@

CFLAGS += -O0
CFLAGS += -Wall -Werror -Wshadow
CFLAGS += -g
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

CONLYFLAGS += -std=gnu99

include $(ROOT_DIR)/make/unittest.mk
