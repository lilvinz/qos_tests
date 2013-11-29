
include $(ROOT_DIR)/make/firmware-defs.mk

BL_ELF := $(BUILD_DIR)/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).elf
FW_BIN := $(BUILD_DIR)/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin
TARGET_BIN := $(OUTDIR)/ef_$(BOARD_NAME).bin
TARGET_ELF := $(OUTDIR)/ef_$(BOARD_NAME).elf

$(OUTDIR):
	$(V1) [ -d $@ ] || mkdir -p $@

.PHONY: all
all: $(TARGET_BIN) $(TARGET_ELF)

FORCE:

# force this target as FW_ORIGIN could have changed
$(TARGET_BIN): $(OUTDIR) FORCE
$(TARGET_BIN): | $(BL_ELF) $(FW_BIN)
	$(V0) @echo $(MSG_PADDING) $(call toprel, $@)
	$(V1) $(OBJCOPY) --pad-to=$(FW_ORIGIN) --gap-fill=0xff -O binary $(BL_ELF) $(TARGET_BIN)
	$(V1) cat $(FW_BIN) >> $(TARGET_BIN)

# force this target as BL_ORIGIN could have changed
$(TARGET_ELF): $(TARGET_BIN) FORCE
	$(V0) @echo $(MSG_FLASH_IMG) $(call toprel, $@)
	$(V1) $(OBJCOPY) -I binary -O elf32-littlearm -B arm --set-start $(BL_ORIGIN) $(TARGET_BIN) $(TARGET_ELF)
