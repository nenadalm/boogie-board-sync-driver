#include <stdlib.h>
#include <stdio.h>
#include <libusb.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <signal.h>
#include "boogieboard.h"

#define ERASE_BTN_FLAG 0x01 << 7
#define SAVE_BTN_FLAG 0x01 << 6
#define ERASE_COMPLETED_FLAG 0x01 << 5
#define SAVE_COMPLETED_FLAG 0x01 << 4
#define STYLUS_HOVER_FLAG 0x01 << 2
#define STYLUS_BTN_FLAG 0x01 << 1
#define STYLUS_TOUCH_FLAG 0x01
#define MAX_X 19780
#define MAX_Y 13442
#define MAX_P 1023

uint8_t should_exit = 0;


void handle_signal(int sig) {
    should_exit = 1;
}

int main() {
    int exit_status = 0;
    signal(SIGINT, handle_signal);

    if (libusb_init(NULL) != 0) {
        fprintf(stderr, "Error when initializing libusb.\n");
        return -1;
    }

    libusb_device_handle* board_handle = libusb_open_device_with_vid_pid(NULL, 0x2914, 0x0100);
    if (board_handle == NULL) {
        fprintf(stderr, "Error when opening BoogieBoard.\n");
        exit_status = 1;
        goto DEINIT_USB;
    }

    if (libusb_kernel_driver_active(board_handle, 1) == 1) {
        libusb_detach_kernel_driver(board_handle, 1);
    }
    if (libusb_kernel_driver_active(board_handle, 0) == 1) {
        libusb_detach_kernel_driver(board_handle, 0);
    }

    if (libusb_claim_interface(board_handle, 0) != 0) {
        fprintf(stderr, "Couldn't claim device.\n");
    }
    if (libusb_claim_interface(board_handle, 1) != 0) {
        fprintf(stderr, "Couldn't claim device.\n");
    }

    uint8_t payload[] = { 0x05, 0x00, 0x03 };
    if (libusb_control_transfer(
                board_handle,
                0x21,
                0x09,
                0x0305,
                1,
                payload,
                3,
                100
                ) <= 0) {
        fprintf(stderr, "Couldn't make transfer to the BoogieBoard.\n");
        exit_status = 1;
        goto CLOSE_HANDLE;
    }

    struct libevdev* dev = libevdev_new();
    boogieboard_configure_device(dev);

    struct libevdev_uinput* uidev;
    if (libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev) != 0) {
        fprintf(stderr, "Couldn't create uinput.\n");
        exit_status = 1;
        goto DESTROY_UINPUT;
    }

    unsigned char data[8];
    int transferred;

    while (!should_exit) {
        if (libusb_interrupt_transfer(
                    board_handle,
                    0x82,
                    data,
                    8,
                    &transferred,
                    500
                    ) != 0) {
            continue;
        }

        boogieboard_process_payload(uidev, data);
    }

DESTROY_UINPUT:
    libevdev_uinput_destroy(uidev);
    libevdev_free(dev);
CLOSE_HANDLE:
    libusb_close(board_handle);
DEINIT_USB:
    libusb_exit(NULL);

    return exit_status;
}
