

COMMON_COMMON_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(COMMON_COMMON_DIR)/*.c)
EXTRAINCDIRS += $(COMMON_COMMON_DIR)

include $(COMMON_COMMON_DIR)/../library.mk
