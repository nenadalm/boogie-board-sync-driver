#ifndef BOOGIEBOARD_H
#define BOOGIEBOARD_H

#include <stdlib.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#define BOOGIEBOARD_ERASE_BTN_FLAG 0x01 << 7
#define BOOGIEBOARD_SAVE_BTN_FLAG 0x01 << 6
#define BOOGIEBOARD_ERASE_COMPLETED_FLAG 0x01 << 5
#define BOOGIEBOARD_SAVE_COMPLETED_FLAG 0x01 << 4
#define BOOGIEBOARD_STYLUS_HOVER_FLAG 0x01 << 2
#define BOOGIEBOARD_STYLUS_BTN_FLAG 0x01 << 1
#define BOOGIEBOARD_STYLUS_TOUCH_FLAG 0x01
#define BOOGIEBOARD_MAX_X 19780
#define BOOGIEBOARD_MAX_Y 13442
#define BOOGIEBOARD_MAX_P 1023

void boogieboard_configure_device(struct libevdev* dev);
void boogieboard_process_payload(struct libevdev_uinput* uidev, unsigned char* data);

#endif
