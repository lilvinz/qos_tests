
include $(ROOT_DIR)/make/firmware-defs.mk

BL_ELF := $(BUILD_DIR)/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).elf
FW_BIN := $(BUILD_DIR)/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin
TARGET_BIN := $(OUTDIR)/ef_$(BOARD_NAME).bin
TARGET_ELF := $(OUTDIR)/ef_$(BOARD_NAME).elf

.PHONY: all
all: $(TARGET_BIN) $(TARGET_ELF)

FORCE:

# Enforce this target as we can't tell if any external dependency has changed
$(TARGET_BIN): FORCE
$(TARGET_BIN): $(BL_ELF) $(FW_ELF) | $(OUTDIR)
	$(V0) @echo $(MSG_PADDING) $(call toprel, $@)
	$(V1) $(OBJCOPY) --pad-to=$(BL_ORIGIN) --gap-fill=0xff -O binary $(BL_ELF) $@
	$(V1) cat $(FW_BIN) >> $@
	$(V1) $(OBJCOPY) --pad-to=$(EF_SIZE) --gap-fill=0xff -I binary -O binary $@ $@

$(TARGET_ELF): $(TARGET_BIN) | $(OUTDIR)
	$(V0) @echo $(MSG_FLASH_IMG) $(call toprel, $@)
	$(V1) $(OBJCOPY) -I binary -O elf32-littlearm -B arm --set-start $(BL_ORIGIN) $< $@

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE,$(TARGET_BIN),$(EF_ORIGIN),$(EF_SIZE),$(OPENOCD_JTAG_CONFIG),$(OPENOCD_CONFIG)))
