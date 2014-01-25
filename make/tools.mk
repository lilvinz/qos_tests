###########################################################################################################
#
# Set up ARM (STM32) SDK
#
###########################################################################################################

ARM_SDK_DIR := $(TOOLS_DIR)/gcc-arm-none-eabi-4_8-2013q4

.PHONY: arm_sdk_install
arm_sdk_install: ARM_SDK_URL := https://launchpad.net/gcc-arm-embedded/4.8/4.8-2013-q4-major/+download/gcc-arm-none-eabi-4_8-2013q4-20131204-linux.tar.bz2
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
openocd_install: OPENOCD_URL := https://github.com/lilvinz/openocd
openocd_install: OPENOCD_REV := v0.7.0_freertos_cm4_stacking
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
	    git apply $(ROOT_DIR)/make/openocd/patchset/0001-freertos-configured-for-v7.6.0-MPU.patch ; \
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

GTEST_DIR := $(TOOLS_DIR)/gtest-1.7.0

.PHONY: gtest_install
gtest_install: | $(DL_DIR) $(TOOLS_DIR)
gtest_install: GTEST_URL := http://googletest.googlecode.com/files/gtest-1.7.0.zip
gtest_install: GTEST_FILE := $(notdir $(GTEST_URL))
gtest_install: gtest_clean
    # download the file unconditionally since google code gives back 404
    # for HTTP HEAD requests which are used when using the wget -N option
	$(V1) [ ! -f "$(DL_DIR)/$(GTEST_FILE)" ] || $(RM) "$(DL_DIR)/$(GTEST_FILE)"
	$(V1) wget -P "$(DL_DIR)" --trust-server-name "$(GTEST_URL)"

    # extract the source
	$(V1) [ ! -d "$(GTEST_DIR)" ] || $(RM) -rf "$(GTEST_DIR)"
	$(V1) mkdir -p "$(GTEST_DIR)"
	$(V1) unzip -q -d "$(TOOLS_DIR)" "$(DL_DIR)/$(GTEST_FILE)"

.PHONY: gtest_clean
gtest_clean:
	$(V0) @echo " CLEAN        $(GTEST_DIR)"
	$(V1) [ ! -d "$(GTEST_DIR)" ] || $(RM) -rf "$(GTEST_DIR)"

###########################################################################################################
#
# Set up astyle tools
#
###########################################################################################################

ASTYLE_DIR := $(TOOLS_DIR)/astyle
ASTYLE_BUILD_DIR := $(DL_DIR)/astyle

.PHONY: astyle_install
astyle_install: | $(DL_DIR) $(TOOLS_DIR)
astyle_install: ASTYLE_URL := https://svn.code.sf.net/p/astyle/code/trunk/AStyle
astyle_install: ASTYLE_REV := 376
astyle_install: ASTYLE_OPTIONS := prefix=$(ASTYLE_DIR)
astyle_install: astyle_clean
    # download the source
	$(V0) @echo " DOWNLOAD     $(ASTYLE_URL) @ $(ASTYLE_REV)"
	$(V1) svn export -q "$(ASTYLE_URL)" -r $(ASTYLE_REV) "$(ASTYLE_BUILD_DIR)"

    # build and install
	$(V0) @echo " BUILD        $(ASTYLE_DIR)"
	$(V1) mkdir -p "$(ASTYLE_DIR)"
	$(V1) ( \
	    $(MAKE) -C $(ASTYLE_BUILD_DIR)/build/gcc $(ASTYLE_OPTIONS) ; \
	    $(MAKE) -C $(ASTYLE_BUILD_DIR)/build/gcc $(ASTYLE_OPTIONS) install ; \
	)

    # delete the extracted source when we're done
	$(V1) [ ! -d "$(ASTYLE_BUILD_DIR)" ] || $(RM) -r "$(ASTYLE_BUILD_DIR)"

.PHONY: astyle_clean
astyle_clean:
	$(V0) @echo " CLEAN        $(ASTYLE_DIR)"
	$(V1) [ ! -d "$(ASTYLE_DIR)" ] || $(RM) -r "$(ASTYLE_DIR)"
	$(V0) @echo " CLEAN        $(ASTYLE_BUILD_DIR)"
	$(V1) [ ! -d "$(ASTYLE_BUILD_DIR)" ] || $(RM) -r "$(ASTYLE_BUILD_DIR)"

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

ifeq ($(shell [ -d "$(ASTYLE_DIR)" ] && echo "exists"), exists)
    export ASTYLE := $(ASTYLE_DIR)/bin/astyle
else
    # not installed, hope it's in the path...
    export ASTYLE ?= astyle
endif

