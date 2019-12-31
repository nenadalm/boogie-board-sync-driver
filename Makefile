LIBS=libusb-1.0 libevdev
CC=gcc

USB_SOURCES := ./src/usb.c ./src/boogieboard.c

DEFAULT: all

bin:
	mkdir bin

all: | bin
	$(CC) $(USB_SOURCES) $$(pkg-config --libs --cflags $(LIBS)) -o ./bin/usb

.PHONY: clean
clean:
	rm -rf ./bin
