
COMMON_BL_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(COMMON_BL_DIR)/*.c)
EXTRAINCDIRS += $(COMMON_BL_DIR)

