# Set up a default goal
.DEFAULT_GOAL := all

ifndef NDEBUG
    PRODUCT_NAME = odcp_$(BOARD_NAME)_DEBUG
else
    PRODUCT_NAME = odcp_$(BOARD_NAME)
endif

.PHONY: all
all: FLASHTOOL_OPTIONS := \
	BUILD_DIR=$(OUTDIR)/ \
	PROGRAMMER=openocd \
	PRODUCT_NAME=$(PRODUCT_NAME) \
	PRODUCT_FILES="$(BUILD_DIR)/ef_$(BOARD_NAME)/ef_$(BOARD_NAME).bin $(BUILD_DIR)/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).bin $(BUILD_DIR)/fw_$(BOARD_NAME)/fw_$(BOARD_NAME).enc ./flashtool/*"
all:
	$(V1) $(MAKE) -r --no-print-directory -f $(ROOT_DIR)/submodules/flashtool/Makefile $(FLASHTOOL_OPTIONS) clean
	$(V1) $(MAKE) -r --no-print-directory -f $(ROOT_DIR)/submodules/flashtool/Makefile $(FLASHTOOL_OPTIONS)
