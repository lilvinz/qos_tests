COMPONENTS_PATH = ../../..
#include $(COMPONENTS_PATH)/Makerules
include $(MAKERULES)

DEBUG = NO

# Hardware flags.
CFLAGS += $(CFLAGS_HW)

##### PATHS FLAGS OF INCLUDES #########
CFLAGS += -I./include
CFLAGS += -I./../include
CFLAGS += -I$(HAL_PATH)/include
CFLAGS += -I$(HAL_HWD_COMMON_PATH)/include
CFLAGS += -I$(SE_PATH)/include

#### DEFINES FLAGS #######
ifeq ($(DEBUG), YES)
  CFLAGS += -DUSB_TRACE
endif

###### LIB ##########
BUILDDIR = .

LIBDIR = ./../lib
LDFLAGS = -L$(LIBDIR)
PROG = VCP.elf
LIBS = $(LIBDIR)/lib$(VCP_LIB).a

driver_label:
	@echo 
	@echo ----------------------------------------------------
	@echo VCP library creation.
	@echo ----------------------------------------------------

modules =             \
       vcpCdcProtocol \
       vcpDescriptors \
       vcpEnumeration \
       vcpVirtualUsart

objects = $(addsuffix .o,$(addprefix $(BUILDDIR)/objs/,$(modules)))
sources = $(addsuffix .c,$(addprefix $(BUILDDIR)/src/,$(modules)))

###### TARGETS ################
all: driver_label LIB_VCP
################
$(BUILDDIR)/objs/%.o: $(BUILDDIR)/src/%.c
	$(CC) $(CFLAGS) $^ -o $@

################
LIB_VCP : $(objects)
	$(AR) $(AR_KEYS) $(LIBDIR)/lib$(VCP_LIB).a $(objects)
	$(SIZE) -td $(LIBDIR)/lib$(VCP_LIB).a
################
clean:
	rm -f $(objects) $(LIBS) $(BUILDDIR)/list/*.*