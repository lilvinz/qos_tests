
include $(ROOT_DIR)/make/firmware-defs.mk

# Flags passed to the preprocessor.
CPPFLAGS += -I$(GTEST_DIR)/include

CFLAGS += -MMD -MP -MF $(OUTDIR)/$(@F).d

# Google Test needs the pthread library
LDFLAGS += -lpthread

#################################
#
# Template to build the unit test
#
#################################

# Set up a default goal
.DEFAULT_GOAL := all

EXTRAINCDIRS += .
ALLSRC := $(CSRC) $(wildcard ./*.c)
ALLCPPSRC := $(wildcard ./*.cpp) $(GTEST_DIR)/src/gtest_main.cc
ALLSRCBASE := $(notdir $(basename $(ALLSRC) $(ALLCPPSRC)))
ALLOBJ := $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

$(foreach src,$(ALLSRC),$(eval $(call COMPILE_C_TEMPLATE,$(src))))

$(foreach src,$(ALLCPPSRC),$(eval $(call COMPILE_CPP_TEMPLATE,$(src))))

# Specific extensions to CPPFLAGS only for the google test library
$(OUTDIR)/gtest-all.o : CPPFLAGS += -I$(GTEST_DIR)
$(eval $(call COMPILE_CPP_TEMPLATE, $(GTEST_DIR)/src/gtest-all.cc))

$(eval $(call LINK_CPP_TEMPLATE,$(OUTDIR)/$(TARGET).elf,$(ALLOBJ) $(OUTDIR)/gtest-all.o))

.PHONY: all
all: elf

.PHONY: elf
elf: $(OUTDIR)/$(TARGET).elf

.PHONY: xml
xml: $(OUTDIR)/$(TARGET).xml

$(OUTDIR)/$(TARGET).xml: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TEST XML    $(MSG_EXTRA) $(call toprel, $@)"
	$(V1) $< --gtest_output=xml:$(OUTDIR)/$(TARGET).xml > /dev/null || [ $$? -eq 1 ]

.PHONY: run
run: $(OUTDIR)/$(TARGET).elf
	$(V0) @echo " TEST RUN    $(MSG_EXTRA) $(call toprel, $<)"
	$(V1) $< || [ $$? -eq 1 ]
