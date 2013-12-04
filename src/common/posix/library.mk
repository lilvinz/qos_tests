

POSIX_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(POSIX_DIR)/*.c)
EXTRAINCDIRS += $(POSIX_DIR)
CDEFS += -DPOSIX

include $(ROOT_DIR)/src/common/library.mk
