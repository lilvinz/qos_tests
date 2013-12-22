

QHAL_STM32_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(QHAL_STM32_DIR)/*.c)
EXTRAINCDIRS += $(QHAL_STM32_DIR)

# ChibiOS
include $(ROOT_DIR)/src/lib/ChibiOS/os/ports/GCC/ARMCMx/STM32F4xx/port.mk

include $(QHAL_STM32_DIR)/../../library.mk
