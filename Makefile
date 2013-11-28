
TARGET_HW ?= TARGET_MULTI
BUILD_TYPE ?= RELEASE_BUILD

SUBDIRS = main/bootloader main

export TARGET_HW
export BUILD_TYPE


all:
	for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1; done

clean:
	for dir in $(SUBDIRS) ; do $(MAKE) clean -C $$dir || exit 1; done

dist:
	for dir in $(SUBDIRS) ; do $(MAKE) dist -C $$dir || exit 1; done
