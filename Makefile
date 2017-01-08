################################################################################
#
# PwrUsbCtl Makefile
#
################################################################################

# CLI Sources ##################################################################

PWRUSBCTL_SRCS = src/main.cpp
PWRUSBCTL_SRCS += src/power_usb_device.cc

# Binary Targets ###############################################################

PWRUSBCTL_BIN = pwrusbctl

# Common Compiler Flags ########################################################

CFLAGS  = -std=c++11
CFLAGS += -Isrc/
CFLAGS += -Wall

# HIDAPI Support ################################################################

# When targetting Linux there are two options given for hidapi, but neither are
# called 'hidapi' which mandates the use of this condition.

HOST_OS = $(shell uname)

ifeq ($(HOST_OS), Linux)
LIBHIDAPI = hidapi-libusb
else
LIBHIDAPI = hidapi
endif

# CLI Compiler Flags ###########################################################

PWRUSBCTL_CFLAGS = $(CFLAGS)
PWRUSBCTL_CFLAGS += `pkg-config --cflags $(LIBHIDAPI)`

# Common Linker Flags ##########################################################

LDFLAGS =

# CLI Linker Flags #############################################################

PWRUSBCTL_LDFLAGS  = $(LDFLAGS)
PWRUSBCTL_LDFLAGS += `pkg-config --libs $(LIBHIDAPI)`

# Build Targets ################################################################

all: $(PWRUSBCTL_BIN)

$(PWRUSBCTL_BIN): $(PWRUSBCTL_SRCS)
	g++ $(PWRUSBCTL_CFLAGS) $(PWRUSBCTL_LDFLAGS) $^ -o $@

run_pwrusbctl: $(PWRUSBCTL_BIN)
	./$(PWRUSBCTL_BIN)

clean:
	rm -f $(PWRUSBCTL_BIN)
