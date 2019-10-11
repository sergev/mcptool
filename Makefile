CC             ?= gcc

VERSION         = $(shell git describe --tags --abbrev=0)
GITCOUNT        = $(shell git rev-list HEAD --count)
UNAME           = $(shell uname)

OBJS            = main.o #util.o hid.o serial.o
CFLAGS         ?= -g -O -Wall -Werror
CFLAGS         += -DVERSION='"$(VERSION).$(GITCOUNT)"' \
                  $(shell pkg-config --cflags libusb-1.0)
LDFLAGS        ?= -g
LIBS            = $(shell pkg-config --libs --static libusb-1.0)

#
# Make sure pkg-config is installed.
#
ifeq ($(shell pkg-config --version),)
    $(error Fatal error: pkg-config is not installed)
endif

#
# Linux
#
# To install required libraries, use:
#   sudo apt-get install pkg-config libusb-1.0-0-dev libudev-dev
#
ifeq ($(UNAME),Linux)
    OBJS        += hid-libusb.o

    # Link libusb statically, when possible
    LIBUSB      = /usr/lib/x86_64-linux-gnu/libusb-1.0.a
    ifeq ($(wildcard $(LIBUSB)),$(LIBUSB))
        LIBS    = $(LIBUSB) -lpthread -ludev
    endif
endif

#
# Mac OS X
#
# To install required libraries, use:
#   brew install pkg-config libusb
#
ifeq ($(UNAME),Darwin)
    OBJS        += hid-macos.o
    LIBS        += -framework IOKit -framework CoreFoundation
endif

all:		mcptool

mcptool:	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
		rm -f *~ *.o core mcptool mcptool.exe

install:	mcptool
		install -c -s mcptool /usr/local/bin/mcptool

###
hid-libusb.o: hid-libusb.c util.h
hid-macos.o: hid-macos.c util.h
hid-windows.o: hid-windows.c util.h
main.o: main.c mcp2221.h util.h
