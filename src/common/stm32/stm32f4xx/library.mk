

STM32F4XX_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(STM32F4XX_DIR)/*.c)
EXTRAINCDIRS += $(STM32F4XX_DIR)

include $(ROOT_DIR)/src/lib/STM32F4xx_StdPeriph_Driver/library.mk
include $(ROOT_DIR)/src/common/stm32/library.mk
