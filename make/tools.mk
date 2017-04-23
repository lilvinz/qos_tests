
###########################################################################################################
#
# Set up Google Test (gtest) tools
#
###########################################################################################################

GTEST_DIR := $(TOOLS_DIR)/googletest-release-1.8.0/googletest

.PHONY: gtest_install
gtest_install: | $(DL_DIR) $(TOOLS_DIR)
gtest_install: GTEST_URL := https://github.com/google/googletest/archive/release-1.8.0.tar.gz
gtest_install: GTEST_FILE := $(notdir $(GTEST_URL))
gtest_install: gtest_clean
    # download the source only if it's newer than what we already have
	$(V1) [ ! -f "$(DL_DIR)/$(GTEST_FILE)" ] || $(RM) "$(DL_DIR)/$(GTEST_FILE)"
	$(V1) wget -N -P "$(DL_DIR)" "$(GTEST_URL)"

    # extract the source
	$(V1) [ ! -d "$(GTEST_DIR)" ] || $(RM) -rf "$(GTEST_DIR)"
	$(V1) tar -C $(TOOLS_DIR) -xzf "$(DL_DIR)/$(GTEST_FILE)"

.PHONY: gtest_clean
gtest_clean:
	$(V0) @echo " CLEAN        $(GTEST_DIR)"
	$(V1) [ ! -d "$(GTEST_DIR)" ] || $(RM) -rf "$(GTEST_DIR)"

