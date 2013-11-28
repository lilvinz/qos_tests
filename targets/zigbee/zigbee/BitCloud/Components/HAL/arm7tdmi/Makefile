HAL_PATH = ..
include $(HAL_PATH)/Makerules

######
LIBDIR = $(STACK_LIB_PATH)
LIB = $(LIBDIR)/lib$(HAL_LIB).a
BUILDDIR = $(HAL_PATH)/arm7tdmi/at91sam7x256

##### PATHS FLAGS OF INCLUDES #########
CFLAGS += -I$(HAL_PATH)/include
CFLAGS += -I$(HAL_HWD_COMMON_PATH)/include
CFLAGS += -I$(HAL_MAC_API_PATH)/include

ifeq ($(COMPILER_TYPE), GCC)
  ifeq ($(OS), FREE_RTOS)
    CFLAGS += -I$(COMPONENTS_PATH)/../FreeRTOS
    CFLAGS += -I$(COMPONENTS_PATH)/../FreeRTOS/include
    BOOT_ASM = $(BUILDDIR)/HAL_HWD/src/halFreertosBoot.s
  endif
  ifeq ($(OS), NONE_OS)
    BOOT_ASM = $(HAL_HWD_COMMON_PATH)/src/halFirmwareBootGcc.s
  endif
endif
ifeq ($(COMPILER_TYPE), IAR)
  ifeq ($(OS), FREE_RTOS)
  endif
  ifeq ($(OS), NONE_OS)
    BOOT_ASM = $(HAL_HWD_COMMON_PATH)/src/halFirmwareBootIar.s
  endif
endif

ifeq ($(COMPILER_TYPE), IAR)
  ASMFLAGS  = -I$(HAL_HWD_COMMON_PATH)/include 
  ASMFLAGS += -S
endif

CFLAGS += -I$(SE_PATH)/include

#### DEFINES FLAGS #######
#CFLAGS += -DHAL_DISABLE_WDT   // WTF? Wanna have WDT! CBA20120106
CFLAGS += -DHAL_USE_TIMER4_COMPA
CFLAGS += -DHAL_USE_SPI0_TXBUFE
CFLAGS += -DHAL_USE_SPI0_RXBUFF
CFLAGS += -DHAL_USE_SPI1_TXBUFE
CFLAGS += -DHAL_USE_SPI1_RXBUFF
CFLAGS += -DHAL_USE_TWI
CFLAGS += -DHAL_USE_ADC
CFLAGS += -DHAL_USE_USB_ENDPOINT
CFLAGS += -DHAL_USE_USB_SUSPEND
CFLAGS += -DHAL_USE_USB_RESUME
CFLAGS += -DHAL_USE_USB_BUS_RESET

##### DECLARES USART CHANNELS TO BE USED ##########
CFLAGS += -DHAL_USE_USART
CFLAGS += -DNUM_USART_CHANNELS=2
CFLAGS += -DHAL_USE_USART_CHANNEL_0
CFLAGS += -DHAL_USE_USART_CHANNEL_1
# Defines which USART uses hardware control pins - only one port at 
# the same time can be hardware controlled. If the macros is not declared - 
# hw control is not used.
# CFLAGS += -DHW_CONTROL_PINS_PORT_ASSIGNMENT=USART_CHANNEL_0

#### FreeRTOS FLAGS #######
ifeq ($(OS), FREE_RTOS)
  CFLAGS += -DSAM7_GCC
  CFLAGS += -DTHUMB_INTERWORK
endif

#### COMPILER FLAGS #######
## ARM core work mode. For other cores is not defined.
THUMBFLAGS=$(CFLAGS)
ARMFLAGS=$(patsubst %$(ARM_MODE_THUMBS_CFLAGS),,$(CFLAGS)) $(ARM_MODE_ARM_CFLAGS)

###### LIB ##########
  common_hwd += atomic
  common_hwd += halAppClock
  common_hwd += halEeprom
  common_hwd += halUsart
  common_hwd += halIrq
  common_hwd += i2c
  common_hwd += wdt
  common_hwd += halAdc
  common_hwd += halSpi
  common_hwd += halInit
  common_hwd += halW1
  common_hwd += halLowLevelInit
  common_hwd += dbgPort
  common_hwd += halUsb

  personal_hwd += halRfCtrl
  personal_hwd += halMacIsr
  personal_hwd += halUid

  hwi += appTimer
  hwi += eeprom
  hwi += halTaskManager
  hwi += usart
  hwi += w1
  hwi += irq
  hwi += resetReason
  hwi += i2cPacket
  hwi += timer
  hwi += adc
  hwi += spi
  hwi += usb
  hwi += sleep

objects_hwd = $(addsuffix .o,$(addprefix $(HAL_HWD_COMMON_PATH)/objs/,$(common_hwd)))
objects_hwd += $(addsuffix .o,$(addprefix $(HAL_MAC_API_PATH)/objs/,$(personal_hwd)))
sources_hwd = $(addsuffix .c,$(addprefix $(HAL_HWD_COMMON_PATH)/src/,$(common_hwd)))
sources_hwd += $(addsuffix .c,$(addprefix $(HAL_MAC_API_PATH)/src/,$(personal_hwd)))
objects_hwi = $(addsuffix .o,$(addprefix $(BUILDDIR)/../common/objs/,$(hwi)))
sources_hwi = $(addsuffix .c,$(addprefix $(BUILDDIR)/../common/src/,$(hwi)))

###### TARGETS ################
all: component_label $(LIB)

component_label:
	@echo 
	@echo ----------------------------------------------------
	@echo HAL library creation.
	@echo ----------------------------------------------------

################
$(BOOT_OBJ):
ifeq ($(COMPILER_TYPE), GCC)
	$(CC_MSG)
	$(Q)$(CC) $(CFLAGS) $^ -o $@
endif
ifeq ($(COMPILER_TYPE), IAR)
	$(AS_MSG)
	$(Q)$(AS) $(ASMFLAGS) $^ -o $@
endif
$(BOOT_OBJ): $(BOOT_ASM)

################ common part ##############################
$(HAL_HWD_COMMON_PATH)/objs/%.o: $(HAL_HWD_COMMON_PATH)/src/%.c
	$(CC_MSG)
	$(Q)$(CC) $(ARMFLAGS) $^ -o $@
################ common part ##############################	

################ personal part ##############################
$(HAL_MAC_API_PATH)/objs/%.o: $(HAL_MAC_API_PATH)/src/%.c
	$(CC_MSG)
	$(Q)$(CC) $(ARMFLAGS) $^ -o $@
################ personal part ##############################

################ hwi part ###################################
$(BUILDDIR)/../common/objs/%.o: $(BUILDDIR)/../common/src/%.c
	$(CC_MSG)
	$(Q)$(CC) $(THUMBFLAGS) $^ -o $@
################ hwi part ###################################

################
$(LIB): $(BOOT_OBJ) $(objects_hwd) $(objects_hwi)
	$(AR_MSG)
	$(Q)$(AR) $(AR_KEYS) $(LIB) $(objects_hwd) $(objects_hwi)
	$(SIZE_MSG)
	$(Q)$(SHOW_SIZE) -td $(BOOT_OBJ)
	$(SIZE_MSG)
	$(Q)$(SHOW_SIZE) -td $(LIB)	

#	$(foreach lib_iter,$(wildcard $(LIBDIR)/lib*$(STACK_LIB)*.a), $(shell $(AR) $(AR_KEYS) $(lib_iter) $(objects_hwd) $(objects_asm_hwd) $(objects_hwi)))
################
clean:
	@echo 
	@echo ----------------------------------------------------
	@echo HAL component cleaning is started...
	$(Q)rm -f $(objects_hwd) $(objects_hwi) $(BOOT_OBJ) $(LIB)
	$(Q)rm -f $(HAL_PATH)/lib/*.a $(HAL_PATH)/lib/*.o $(HAL_PATH)/list/*.*
	@echo HAL component cleaning is done!
	@echo ----------------------------------------------------
