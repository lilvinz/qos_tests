###########################################################################################################
#
# Set up ARM (STM32) SDK
#
###########################################################################################################

ARM_SDK_DIR := $(TOOLS_DIR)/gcc-arm-none-eabi-6-2017-q1-update

.PHONY: arm_sdk_install
arm_sdk_install: ARM_SDK_URL := https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/6_1-2017q1/gcc-arm-none-eabi-6-2017-q1-update-linux.tar.bz2
arm_sdk_install: ARM_SDK_FILE := $(notdir $(ARM_SDK_URL))
# order-only prereq on directory existance:
arm_sdk_install: | $(DL_DIR) $(TOOLS_DIR)
arm_sdk_install: arm_sdk_clean
    # download the source only if it's newer than what we already have
	$(V1) wget --no-check-certificate -N -P "$(DL_DIR)" "$(ARM_SDK_URL)"

    # binary only release so just extract it
	$(V1) tar -C $(TOOLS_DIR) -xjf "$(DL_DIR)/$(ARM_SDK_FILE)"

.PHONY: arm_sdk_clean
arm_sdk_clean:
	$(V1) [ ! -d "$(ARM_SDK_DIR)" ] || $(RM) -r $(ARM_SDK_DIR)

###########################################################################################################
#
# Set up openocd tools
#
###########################################################################################################

OPENOCD_DIR := $(TOOLS_DIR)/openocd
OPENOCD_BUILD_DIR := $(DL_DIR)/openocd-build

.PHONY: openocd_install
openocd_install: | $(DL_DIR) $(TOOLS_DIR)
openocd_install: OPENOCD_URL := git://git.code.sf.net/p/openocd/code
openocd_install: OPENOCD_REV := 888d5a5ec9f0e2178241c7fda99558d2e33a06a5
openocd_install: OPENOCD_OPTIONS := --enable-maintainer-mode --prefix="$(OPENOCD_DIR)" --enable-ftdi --enable-stlink
openocd_install: openocd_clean
    # download the source
	$(V0) @echo " DOWNLOAD     $(OPENOCD_URL) @ $(OPENOCD_REV)"
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -r "$(OPENOCD_BUILD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_BUILD_DIR)"
	$(V1) git clone --no-checkout $(OPENOCD_URL) "$(OPENOCD_BUILD_DIR)"
	$(V1) ( \
	    cd $(OPENOCD_BUILD_DIR) ; \
	    git checkout -q $(OPENOCD_REV) ; \
	)

    # build and install
	$(V0) @echo " BUILD        $(OPENOCD_DIR)"
	$(V1) mkdir -p "$(OPENOCD_DIR)"
	$(V1) ( \
	    cd $(OPENOCD_BUILD_DIR) ; \
	    ./bootstrap ; \
	    ./configure  $(OPENOCD_OPTIONS) ; \
	    $(MAKE) ; \
	    $(MAKE) install ; \
	)

    # delete the extracted source when we're done
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -r "$(OPENOCD_BUILD_DIR)"

.PHONY: openocd_clean
openocd_clean:
	$(V0) @echo " CLEAN        $(OPENOCD_DIR)"
	$(V1) [ ! -d "$(OPENOCD_DIR)" ] || $(RM) -r "$(OPENOCD_DIR)"
	$(V0) @echo " CLEAN        $(OPENOCD_BUILD_DIR)"
	$(V1) [ ! -d "$(OPENOCD_BUILD_DIR)" ] || $(RM) -r "$(OPENOCD_BUILD_DIR)"

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

###########################################################################################################
#
# Install required package dependencies
#
###########################################################################################################

.PHONY: deps_install
deps_install: DEPS_PACKAGES := build-essential unzip wget bzip2 tar bash gcc-multilib g++-multilib libxcb-image0-dev:i386 libxcb-icccm4-dev:i386 libxcb-util0-dev:i386
deps_install:
	$(V0) @echo " INSTALL      $(DEPS_PACKAGES)"
	$(V1) apt-get install $(DEPS_PACKAGES)

###########################################################################################################
#
# Set up paths to tools
#
###########################################################################################################

ifeq ($(shell [ -d "$(ARM_SDK_DIR)" ] && echo "exists"), exists)
    export ARM_SDK_PREFIX := $(ARM_SDK_DIR)/bin/arm-none-eabi-
else
    # not installed, hope it's in the path...
    export ARM_SDK_PREFIX ?= arm-none-eabi-
endif

ifeq ($(shell [ -d "$(OPENOCD_DIR)" ] && echo "exists"), exists)
    export OPENOCD := $(OPENOCD_DIR)/bin/openocd
else
    # not installed, hope it's in the path...
    export OPENOCD ?= openocd
endif
