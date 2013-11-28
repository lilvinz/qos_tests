#==============================================================================
# System settings
# Modify to your needs.
#==============================================================================

#==============================================================================
# Targets
#==============================================================================

TARGET_FILE		= zigbee_bsl.elf

BOOTLOADER_DIR 	= bootloader
BOOTLOADER_NAME = bootloader
BOOTLOADER_ORG	= 0x100000
FIRMWARE_DIR 	= zigbee

FIRMWARE_SRC_NAME = zigbee

ifeq ($(HAL_ANT_DIVERSITY),TRUE)
  FIRMWARE_NAME	= zigbee_diversity
else
  FIRMWARE_NAME	= zigbee
endif

FIRMWARE_ORG	= 0x104000

IMAGE_INFORMATION_OBJ = $(FIRMWARE_DIR)/makefiles/zigbee/All_Sam7xEk_At91sam7x256_Rf231_Gcc/Obj/image_information.o
IMAGE_INFORMATION_OBJ += $(FIRMWARE_DIR)/makefiles/zigbee/All_Sec_Sam7xEk_At91sam7x256_Rf231_Gcc/Obj/image_information.o


# LFSR encryption parameters
LFSR_TOOL = ./tools/lfsr.pl
LFSR_GENERATOR_POLY = 0xBE4F7253
LFSR_KEY = 0x3FC01A44

PROGRAMMERCOPYDIR = ../../firmware/programmer/firmware/
BINCOPYDIR = ../../firmware/

#-Compiler type definition---------------------
COMPILER_TYPE = GCC
COMPILER_AND_MICRO_TYPE = GCC_ARM
#-Tools definitions----------------------------
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump


all: bsl enc end

bsl: $(shell mkdir -p zigbee/BitCloud/Components/HAL/arm7tdmi/at91sam7x256/common/objs)
bsl: $(shell mkdir -p zigbee/BitCloud/Components/HAL/arm7tdmi/at91sam7x256/atmlEvalKit/objs)
bsl: $(shell mkdir -p zigbee/BitCloud/Components/HAL/arm7tdmi/common/objs)
bsl:
	@echo
	@echo ------------ Creating Bootloader ------------
	make all -C $(BOOTLOADER_DIR)

	@echo
	@echo ------------ Creating Zigbee firmware ------------
	make all -C $(FIRMWARE_DIR)/BitCloud/Components/HAL
	make all -C $(FIRMWARE_DIR)
	# rebuild so the size from previous build can be included in image information.
	@echo Recreating Firmware to include right ImageInformations
	@echo Removing old informations: $(IMAGE_INFORMATION_OBJ)
	rm -f $(IMAGE_INFORMATION_OBJ)
	make all -C $(FIRMWARE_DIR)
	
	@echo
	@echo ------------ Creating bootloader and firmware compound ------------
	rm -f $(FIRMWARE_NAME)_bsl.bin

	$(OBJCOPY) --pad-to=$(FIRMWARE_ORG) -O binary $(BOOTLOADER_DIR)/$(BOOTLOADER_NAME).elf $(BOOTLOADER_NAME).bin
	$(OBJCOPY) -O binary $(FIRMWARE_DIR)/$(FIRMWARE_SRC_NAME).elf $(FIRMWARE_NAME).bin
	cat $(BOOTLOADER_NAME).bin > $(FIRMWARE_NAME)_bsl.bin
	cat $(FIRMWARE_NAME).bin >> $(FIRMWARE_NAME)_bsl.bin
	rm -f $(BOOTLOADER_NAME).bin $(FIRMWARE_NAME).bin
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm --set-start 0x00100000 $(FIRMWARE_NAME)_bsl.bin $(FIRMWARE_NAME)_bsl.elf


enc:
	@echo
	@echo ------------ Encrypting binary file ------------
	perl $(LFSR_TOOL) $(LFSR_GENERATOR_POLY) $(LFSR_KEY) < $(FIRMWARE_DIR)/$(FIRMWARE_SRC_NAME).bin > $(FIRMWARE_NAME).bin.enc || exit 0	


clean:
	make clean -C $(BOOTLOADER_DIR)
	make clean -C $(FIRMWARE_DIR)
	make clean -C $(FIRMWARE_DIR)/BitCloud/Components/HAL
	rm -f $(FIRMWARE_FILE).bin.enc
	rm -f $(FIRMWARE_FILE)_bsl.bin


end:
	@echo 
	@echo ------------ DONE ------------


.PHONY: all clean bsl

.PHONY: flashtool
ifeq ($(HAL_ANT_DIVERSITY),TRUE)
    flashtool: FLASHTOOL_OPTIONS := TOP=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))/ BUILD_DIR=./ PROGRAMMER=openocd PRODUCT_NAME=sirius_r2_$(FIRMWARE_NAME) PRODUCT_FILES="$(FIRMWARE_NAME)_bsl.bin $(FIRMWARE_NAME).bin.enc flashtool_diversity/*"
else
    flashtool: FLASHTOOL_OPTIONS := TOP=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))/ BUILD_DIR=./ PROGRAMMER=openocd PRODUCT_NAME=sirius_r2_$(FIRMWARE_NAME) PRODUCT_FILES="$(FIRMWARE_NAME)_bsl.bin $(FIRMWARE_NAME).bin.enc flashtool/*"
endif
flashtool: all
	$(MAKE) -f ../submodules/flashtool/Makefile $(FLASHTOOL_OPTIONS) clean
	$(MAKE) -f ../submodules/flashtool/Makefile $(FLASHTOOL_OPTIONS)

