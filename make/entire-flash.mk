
###############################################################################
# This makefile requires bash
###############################################################################

WHEREAMI := $(dir $(lastword $(MAKEFILE_LIST)))
TOP ?= $(realpath $(WHEREAMI)/../)
include $(TOP)/make/firmware-defs.mk
include $(TOP)/src/targets/$(BOARD_NAME)/common/board-info.mk

toprel = $(subst $(realpath $(TOP))/,,$(abspath $(1)))

FORCE:

.PHONY: bin
bin: $(OUTDIR)/$(TARGET).bin

BL_BIN := $(TOP)/build/bl_$(BOARD_NAME)/bl_$(BOARD_NAME).bin
FW_BIN := $(TOP)/build/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin
FWINFO_BIN := $(FW_BIN).firmwareinfo.bin

# force this target as FW_PRE_PAD could have been changed without us knowing
FW_PRE_PAD := $(shell echo $$[$(FW_BANK_BASE)-$(BL_BANK_BASE)-$(BL_BANK_SIZE)])
$(OUTDIR)/$(TARGET).fw_pre.pad: FORCE
	$(V0) @echo $(MSG_PADDING) $(call toprel, $@)
	$(V1) dd if=/dev/zero count=$(FW_PRE_PAD) bs=1 2> /dev/null | tr '\000' '\377' > $@ && [ $${PIPESTATUS[0]} -eq "0" ]

# force this target as FW_POST_PAD could have been changed without us knowing
FW_POST_PAD := $(shell echo $$[$(FW_BANK_SIZE)-$(FW_DESC_SIZE)-$(FW_DESC_SIZE)-$(shell wc -c < $(FW_BIN))])
$(OUTDIR)/$(TARGET).fw_post.pad: FORCE
	$(V0) @echo $(MSG_PADDING) $(call toprel, $@)
	$(V1) dd if=/dev/zero count=$(FW_POST_PAD) bs=1 2> /dev/null | tr '\000' '\377' > $@ && [ $${PIPESTATUS[0]} -eq "0" ]

# force this target as $(BL_BIN), $(FW_BIN) and $(FWINFO_BIN) could have been changed without us knowing
# add $(FWINFO_BIN) after $(FW_BIN) to imitate flash contents after manual firmware upload via gcs
$(OUTDIR)/$(TARGET).bin: $(BL_BIN) $(FW_BIN) $(OUTDIR)/$(TARGET).fw_pre.pad $(OUTDIR)/$(TARGET).fw_post.pad FORCE
	$(V0) @echo $(MSG_FLASH_IMG) $(call toprel, $@)
	$(V1) cat $(BL_BIN) $(OUTDIR)/$(TARGET).fw_pre.pad $(FW_BIN) $(FWINFO_BIN) $(OUTDIR)/$(TARGET).fw_post.pad $(FWINFO_BIN) > $@

# Add jtag targets (program and wipe)
$(eval $(call JTAG_TEMPLATE,$(OUTDIR)/$(TARGET).bin,$(EF_BANK_BASE),$(EF_BANK_SIZE),$(OPENOCD_JTAG_CONFIG),$(OPENOCD_CONFIG)))

