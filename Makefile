LIBS=libusb-1.0 libevdev
CC=gcc

DEFAULT: all

bin:
	mkdir bin

all: | bin
	$(CC) ./src/usb.c $$(pkg-config --libs --cflags $(LIBS)) -o ./bin/usb

