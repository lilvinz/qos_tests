

COMMON_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(COMMON_DIR)/*.c)
EXTRAINCDIRS += $(COMMON_DIR)
