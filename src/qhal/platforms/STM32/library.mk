

QHAL_STM32_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CSRC += $(wildcard $(QHAL_STM32_DIR)/*.c)
EXTRAINCDIRS += $(QHAL_STM32_DIR)

# tell ChibiOS that we are using thumb mode as STM32 does not support arm mode at all
CFLAGS += -DTHUMB

# ChibiOS
include $(ROOT_DIR)/src/lib/ChibiOS/os/ports/GCC/ARMCMx/STM32F4xx/port.mk

include $(QHAL_STM32_DIR)/../../library.mk
