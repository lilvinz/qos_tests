
QHAL_STM32F4XX_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(QHAL_STM32F4XX_DIR)/*.c)
EXTRAINCDIRS += $(QHAL_STM32F4XX_DIR)

LDFLAGS += -T$(QHAL_STM32F4XX_DIR)/ld/sections.ld

# ChibiOS
include $(ROOT_DIR)/src/lib/ChibiOS/os/hal/platforms/STM32F4xx/platform.mk

include $(QHAL_STM32F4XX_DIR)/../STM32/library.mk
