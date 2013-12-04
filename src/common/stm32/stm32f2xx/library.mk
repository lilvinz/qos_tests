

STM32F2XX_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(STM32F2XX_DIR)/*.c)
EXTRAINCDIRS += $(STM32F2XX_DIR)

include $(ROOT_DIR)/src/lib/STM32F2xx_StdPeriph_Driver/library.mk
include $(ROOT_DIR)/src/common/stm32/library.mk
