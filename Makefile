# Set up a default goal
.DEFAULT_GOAL := help

# Set bash shell
SHELL := /bin/bash

# Set up some macros for common directories within the tree
export ROOT_DIR := $(CURDIR)
export TARGETS_DIR := $(ROOT_DIR)/src/targets
export TESTS_DIR := $(ROOT_DIR)/src/tests
export TOOLS_DIR := $(ROOT_DIR)/tools
export BUILD_DIR := $(ROOT_DIR)/build
export DL_DIR := $(ROOT_DIR)/downloads

$(TOOLS_DIR):
	$(V1) mkdir -p $@

$(BUILD_DIR):
	$(V1) mkdir -p $@

$(DL_DIR):
	$(V1) mkdir -p $@

# Decide on a verbosity level based on the V= parameter
export AT := @

ifndef V
    export V0 :=
    export V1 := $(AT)
else ifeq ($(V), 0)
    export V0 := $(AT)
    export V1 := $(AT)
else ifeq ($(V), 1)
endif

# Set build variant defaults
export BUILD_TYPE ?= DEBUG_BUILD

ifeq ($(BUILD_TYPE),DEBUG_BUILD)
else ifeq ($(BUILD_TYPE),RELEASE_BUILD)
    export NDEBUG := 1
else
    $(error BUILD_TYPE is not set properly)
endif

# When building any of the "all_*" targets, tell all sub makefiles to display
# additional details on each line of output to describe which build and target
# that each line applies to.
ifneq ($(strip $(filter all_%,$(MAKECMDGOALS))),)
    export ENABLE_MSG_EXTRA := yes
endif

# When building more than one goal in a single make invocation, also
# enable the extra context for each output line
ifneq ($(word 2,$(MAKECMDGOALS)),)
    export ENABLE_MSG_EXTRA := yes
endif

include $(ROOT_DIR)/make/tools.mk

.PHONY: help
help:
	@echo
	@echo "   Here is a summary of the available targets:"
	@echo
	@echo "   [Tools]"
	@echo "     arm_sdk_install      - Install the GNU ARM gcc toolchain"
	@echo "     openocd_install      - Install the OpenOCD JTAG daemon"
	@echo "     gtest_install        - Install the google test framework"
	@echo "     deps_install         - Install required os dependencies"
	@echo
	@echo "   [Big Hammer]"
	@echo "     all                  - Build all firmware, bootloaders, entire flash and flashtool"
	@echo "     all_fw               - Build only firmware for all boards"
	@echo "     all_bl               - Build only bootloaders for all boards"
	@echo "     all_ef               - Build only entire flash imanges for all boards"
	@echo "     all_ft               - Build only flashtool packages for all boards"
	@echo
	@echo "     all_clean            - Remove your build directory ($(BUILD_DIR))"
	@echo "     all_fw_clean         - Remove firmware for all boards"
	@echo "     all_bl_clean         - Remove bootloaders for all boards"
	@echo "     all_ef_clean         - Remove entire flash images for all boards"
	@echo "     all_ft_clean         - Remove flashtool packages for all boards"
	@echo
	@echo "     all_<board>          - Build all available images for <board>"
	@echo "     all_<board>_clean    - Remove all available images for <board>"
	@echo
	@echo "   [Firmware]"
	@echo "     fw_<board>           - Build firmware for <board>"
	@echo "                            supported boards are ($(FW_BOARDS))"
	@echo "     fw_<board>_clean     - Remove firmware for <board>"
	@echo
	@echo "   [Bootloader]"
	@echo "     bl_<board>           - Build bootloader for <board>"
	@echo "                            supported boards are ($(BL_BOARDS))"
	@echo "     bl_<board>_clean     - Remove bootloader for <board>"
	@echo
	@echo "   [Entire Flash]"
	@echo "     ef_<board>           - Build entire flash image for <board>"
	@echo "                            supported boards are ($(EF_BOARDS))"
	@echo "     ef_<board>_clean     - Remove entire flash image for <board>"
	@echo
	@echo "   [Flashtool]"
	@echo "     ft_<board>           - Build flashtool package for <board>"
	@echo "                            supported boards are ($(FT_BOARDS))"
	@echo "     ft_<board>_clean     - Remove flashtool package for <board>"
	@echo
	@echo "   [Unittests]"
	@echo "     ut_<test>            - Build unit test <test>"
	@echo "                            supported tests are ($(ALL_UNITTESTS))"
	@echo "     ut_<test>_xml        - Run test and capture XML output into a file"
	@echo "     ut_<test>_run        - Run test and dump output to console"
	@echo
	@echo "   Hint: Add V=1 to your command line to see verbose build output."
	@echo
	@echo "   Note: All tools will be installed into $(TOOLS_DIR)"
	@echo "         All build output will be placed in $(BUILD_DIR)"
	@echo

.PHONY: all_clean
all_clean:
	$(V0) @echo " CLEAN        $(BUILD_DIR)"
	$(V1) [ ! -d "$(BUILD_DIR)" ] || $(RM) -r "$(BUILD_DIR)"

##############################
#
# embedded related components
#
##############################

# $(1) = Canonical board name all in lower case (e.g. discoveryf4)
define FW_TEMPLATE
.PHONY: $(1) fw_$(1)
fw_$(1): fw_$(1)_all

fw_$(1)_%:
	$(V1) cd $(TARGETS_DIR)/$(1)/fw && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BUILD_PREFIX=fw \
		OUTDIR=$(BUILD_DIR)/fw_$(1) \
		$$*

.PHONY: $(1)_clean
$(1)_clean: fw_$(1)_clean
fw_$(1)_clean:
	$(V0) @echo " CLEAN        $$@"
	$(V1) $(RM) -r $(BUILD_DIR)/fw_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. discoveryf4)
define BL_TEMPLATE
.PHONY: bl_$(1)
bl_$(1): bl_$(1)_all

bl_$(1)_%:
	$(V1) cd $(TARGETS_DIR)/$(1)/bl && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BUILD_PREFIX=bl \
		OUTDIR=$(BUILD_DIR)/bl_$(1) \
		$$*

.PHONY: bl_$(1)_clean
bl_$(1)_clean:
	$(V0) @echo " CLEAN        $$@"
	$(V1) $(RM) -r $(BUILD_DIR)/bl_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. discoveryf4)
define EF_TEMPLATE
.PHONY: ef_$(1)
ef_$(1): ef_$(1)_all

ef_$(1)_%:
	$(V1) cd $(TARGETS_DIR)/$(1)/ef && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BUILD_PREFIX=ef \
		OUTDIR=$(BUILD_DIR)/ef_$(1) \
		$$*

.PHONY: ef_$(1)_clean
ef_$(1)_clean:
	$(V0) @echo " CLEAN        $$@"
	$(V1) $(RM) -r $(BUILD_DIR)/ef_$(1)
endef

# $(1) = Canonical board name all in lower case (e.g. discoveryf4)
define FT_TEMPLATE
.PHONY: ft_$(1)
ft_$(1): ft_$(1)_all

ft_$(1)_%:
	$(V1) cd $(TARGETS_DIR)/$(1)/ft && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BUILD_PREFIX=ft \
		OUTDIR=$(BUILD_DIR)/ft_$(1) \
		$$*

.PHONY: ft_$(1)_clean
ft_$(1)_clean:
	$(V0) @echo " CLEAN        $$@"
	$(V1) $(RM) -r $(BUILD_DIR)/ft_$(1)
endef

# When building any of the "all_*" targets, tell all sub makefiles to display
# additional details on each line of output to describe which build and target
# that each line applies to.
ifneq ($(strip $(filter all%,$(MAKECMDGOALS))),)
    export ENABLE_MSG_EXTRA := yes
endif

# When building more than one goal in a single make invocation, also
# enable the extra context for each output line
ifneq ($(word 2,$(MAKECMDGOALS)),)
    export ENABLE_MSG_EXTRA := yes
endif

# $(1) = Canonical board name all in lower case (e.g. discoveryf4)
define BOARD_PHONY_TEMPLATE
.PHONY: all_$(1)
all_$(1): $$(filter fw_$(1), $$(FW_TARGETS))
all_$(1): $$(filter bl_$(1), $$(BL_TARGETS))
all_$(1): $$(filter ef_$(1), $$(EF_TARGETS))
all_$(1): $$(filter ft_$(1), $$(FT_TARGETS))

.PHONY: all_$(1)_clean
all_$(1)_clean: $$(addsuffix _clean, $$(filter fw_$(1), $$(FW_TARGETS)))
all_$(1)_clean: $$(addsuffix _clean, $$(filter bl_$(1), $$(BL_TARGETS)))
all_$(1)_clean: $$(addsuffix _clean, $$(filter ef_$(1), $$(EF_TARGETS)))
all_$(1)_clean: $$(addsuffix _clean, $$(filter ft_$(1), $$(FT_TARGETS)))
endef

# Include all board definitions from targets dir
-include $(ROOT_DIR)/src/targets/*/target-defs.mk

# Generate the targets
FW_TARGETS := $(addprefix fw_, $(FW_BOARDS))
BL_TARGETS := $(addprefix bl_, $(BL_BOARDS))
EF_TARGETS := $(addprefix ef_, $(EF_BOARDS))
FT_TARGETS := $(addprefix ft_, $(FT_BOARDS))

.PHONY: all_fw all_fw_clean
all_fw: $(FW_TARGETS)
all_fw_clean: $(addsuffix _clean, $(FW_TARGETS))

.PHONY: all_bl all_bl_clean
all_bl: $(BL_TARGETS)
all_bl_clean: $(addsuffix _clean, $(BL_TARGETS))

.PHONY: all_ef all_ef_clean
all_ef: $(EF_TARGETS)
all_ef_clean: $(addsuffix _clean, $(EF_TARGETS))

.PHONY: all_ft all_ft_clean
all_ft: $(FT_TARGETS)
all_ft_clean: $(addsuffix _clean, $(FT_TARGETS))

.PHONY: all
all: all_fw all_bl all_ef all_ft

# Expand the firmware rules
$(foreach board, $(FW_BOARDS), $(eval $(call BOARD_PHONY_TEMPLATE,$(board))))
$(foreach board, $(FW_BOARDS), $(eval $(call FW_TEMPLATE,$(board))))

# Expand the bootloader rules
$(foreach board, $(BL_BOARDS), $(eval $(call BOARD_PHONY_TEMPLATE,$(board))))
$(foreach board, $(BL_BOARDS), $(eval $(call BL_TEMPLATE,$(board))))

# Expand the entire-flash rules
$(foreach board, $(EF_BOARDS), $(eval $(call BOARD_PHONY_TEMPLATE,$(board))))
$(foreach board, $(EF_BOARDS), $(eval $(call EF_TEMPLATE,$(board))))

# Expand the flashtool rules
$(foreach board, $(FT_BOARDS), $(eval $(call BOARD_PHONY_TEMPLATE,$(board))))
$(foreach board, $(FT_BOARDS), $(eval $(call FT_TEMPLATE,$(board))))

##############################
#
# Unit Tests
#
##############################

ALL_UNITTESTS := $(notdir $(wildcard $(TESTS_DIR)/*))

.PHONY: all_ut
all_ut: $(addsuffix _elf, $(addprefix ut_, $(ALL_UNITTESTS)))

.PHONY: all_ut_xml
all_ut_xml: $(addsuffix _xml, $(addprefix ut_, $(ALL_UNITTESTS)))

.PHONY: all_ut_run
all_ut_run: $(addsuffix _run, $(addprefix ut_, $(ALL_UNITTESTS)))

.PHONY: all_ut_clean
all_ut_clean: $(addsuffix _clean, $(addprefix ut_, $(ALL_UNITTESTS)))

# $(1) = Unit test name
define UT_TEMPLATE
.PHONY: ut_$(1)
ut_$(1): ut_$(1)_all

ut_$(1)_%:
	$(V1) cd $(TESTS_DIR)/$(1) && \
		$$(MAKE) -r --no-print-directory \
		BOARD_NAME=$(1) \
		BUILD_PREFIX=ut \
		TCHAIN_PREFIX="" \
		TARGET=ut_$(1) \
		OUTDIR=$(BUILD_DIR)/ut_$(1) \
		GTEST_DIR=$(GTEST_DIR) \
		$$*

.PHONY: ut_$(1)_clean
ut_$(1)_clean:
	$(V0) @echo " CLEAN        $$@"
	$(V1) $(RM) -r $(BUILD_DIR)/ut_$(1)
endef

# Expand the unittest rules
$(foreach ut, $(ALL_UNITTESTS), $(eval $(call UT_TEMPLATE,$(ut))))

# Disable parallel make when the all_ut_run target is requested otherwise the XML
# output is interleaved with the rest of the make output.
ifneq ($(strip $(filter all_ut_run,$(MAKECMDGOALS))),)
    .NOTPARALLEL:
    $(info *NOTE*        Parallel make disabled by all_ut_run target so we have sane console output)
endif

