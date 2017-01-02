################################################################################
#
# PwrUsbCtl Makefile
#
################################################################################

# CLI Sources ##################################################################

PWRUSBCTL_SRCS = src/main.cpp

# Binary Targets ###############################################################

PWRUSBCTL_BIN = pwrusbctl

# Common Compiler Flags ########################################################

CFLAGS  = -std=c++11
CFLAGS += -Isrc/
CFLAGS += -Wall

# CLI Compiler Flags ###########################################################

PWRUSBCTL_CFLAGS = $(CFLAGS)

# Common Linker Flags ##########################################################

LDFLAGS =

# CLI Linker Flags #############################################################

PWRUSBCTL_LDFLAGS  = $(LDFLAGS)

# Build Targets ################################################################

all: $(PWRUSBCTL_BIN)

$(PWRUSBCTL_BIN): $(PWRUSBCTL_SRCS)
	g++ $(PWRUSBCTL_CFLAGS) $(PWRUSBCTL_LDFLAGS) $^ -o $@

run_pwrusbctl: $(PWRUSBCTL_BIN)
	./$(PWRUSBCTL_BIN)

clean:
	rm -f $(PWRUSBCTL_BIN)
