
include $(ROOT_DIR)/make/firmware-defs.mk

BL_BIN := $(BUILD_DIR)/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).bin
FW_BIN := $(BUILD_DIR)/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin
TARGET_BIN := $(OUTDIR)/$(BUILD_PREFIX)_$(BOARD_NAME).bin
TARGET_ELF := $(OUTDIR)/$(BUILD_PREFIX)_$(BOARD_NAME).elf

.PHONY: all
all: $(TARGET_BIN) $(TARGET_ELF)



# Enforce this target as we can't tell if any external dependency has changed
.PHONY: $(TARGET_BIN)
$(TARGET_BIN): $(BL_ELF) $(BG_ELF) $(FW_ELF) | $(OUTDIR)
	$(V0) @echo $(MSG_PADDING) $(call toprel, $@)
	$(V1) cat $(BL_BIN) > $@
	$(V1) $(OBJCOPY) --pad-to=$$(($(FW_ORIGIN) - $(EF_ORIGIN))) --gap-fill=0xff -I binary -O binary $@ $@
	$(V1) cat $(FW_BIN) >> $@
	$(V1) $(OBJCOPY) --pad-to=$(EF_SIZE) --gap-fill=0xff -I binary -O binary $@ $@

$(TARGET_ELF): $(TARGET_BIN) | $(OUTDIR)
	$(V0) @echo $(MSG_FLASH_IMG) $(call toprel, $@)
	$(V1) $(OBJCOPY) -I binary -O elf32-littlearm -B arm --set-start $(BL_ORIGIN) $< $@

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE, $(TARGET_BIN), $(EF_ORIGIN), $(EF_SIZE), $(OPENOCD_JTAG_CONFIG), $(OPENOCD_TRANSPORT), $(OPENOCD_CONFIG)))
