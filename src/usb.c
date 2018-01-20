#include <stdlib.h>
#include <stdio.h>
#include <libusb.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <signal.h>

uint8_t should_exit = 0;


void handle_signal(int sig) {
    should_exit = 1;
}

int clamp(int min, int max, int v) {
    if (v <= min) return min;
    if (v >= max) return max;

    return v;
}

int main() {
    int exit_status = 0;
    signal(SIGINT, handle_signal);

    if (libusb_init(NULL) != 0) {
        fprintf(stderr, "Error when initializing libusb.\n");
        return -1;
    }

    libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_DEBUG);

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
    libevdev_set_name(dev, "BoogieBoard Sync");

    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOUCH, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_STYLUS2, NULL);

    struct input_absinfo abs_info_x;
    abs_info_x.value = 0;
    abs_info_x.minimum = 0;
    abs_info_x.maximum = 19780;
    abs_info_x.fuzz = 0;
    abs_info_x.flat = 0;
    abs_info_x.resolution = 0;

    struct input_absinfo abs_info_y;
    abs_info_y.value = 0;
    abs_info_y.minimum = 0;
    abs_info_y.maximum = 13442;
    abs_info_y.fuzz = 0;
    abs_info_y.flat = 0;
    abs_info_y.resolution = 0;

    struct input_absinfo abs_info_p;
    abs_info_p.value = 0;
    abs_info_p.minimum = 0;
    abs_info_p.maximum = 255;
    abs_info_p.fuzz = 0;
    abs_info_p.flat = 0;
    abs_info_p.resolution = 0;

    libevdev_enable_event_type(dev, EV_ABS);
    libevdev_enable_event_code(dev, EV_ABS, ABS_PRESSURE, &abs_info_p);
    libevdev_enable_event_code(dev, EV_ABS, ABS_X, &abs_info_x);
    libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &abs_info_y);

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

        int xpos = clamp(0, 19780, data[1] | data[2] << 8);
        int ypos = clamp(0, 13442, data[3] | data[4] << 8);
        int pressure = data[5] | data[6] << 8;
        int touch = data[7] & 0x01;
        int stylus = (data[7] & 0x02) >> 1;

        libevdev_uinput_write_event(uidev, EV_ABS, ABS_PRESSURE, pressure);
        libevdev_uinput_write_event(uidev, EV_ABS, ABS_X, xpos);
        libevdev_uinput_write_event(uidev, EV_ABS, ABS_Y, ypos);
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOUCH, touch);
        libevdev_uinput_write_event(uidev, EV_KEY, BTN_STYLUS2, stylus);
        libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
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