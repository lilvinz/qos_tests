

STM32_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(STM32_DIR)/*.c)
EXTRAINCDIRS += $(STM32_DIR)

include $(ROOT_DIR)/src/common/library.mk
