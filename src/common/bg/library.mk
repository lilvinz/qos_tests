

COMMON_BG_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(COMMON_BG_DIR)/*.c)
EXTRAINCDIRS += $(COMMON_BG_DIR)
