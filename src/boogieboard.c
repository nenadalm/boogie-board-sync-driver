#include "boogieboard.h"

void boogieboard_configure_device(struct libevdev* dev) {
    libevdev_set_name(dev, "BoogieBoard Sync");

    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_PEN, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOUCH, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_STYLUS, NULL);

    struct input_absinfo abs_info_x;
    abs_info_x.value = 0;
    abs_info_x.minimum = 0;
    abs_info_x.maximum = BOOGIEBOARD_MAX_X;
    abs_info_x.fuzz = 0;
    abs_info_x.flat = 0;
    abs_info_x.resolution = 1;

    struct input_absinfo abs_info_y;
    abs_info_y.value = 0;
    abs_info_y.minimum = 0;
    abs_info_y.maximum = BOOGIEBOARD_MAX_Y;
    abs_info_y.fuzz = 0;
    abs_info_y.flat = 0;
    abs_info_y.resolution = 1;

    struct input_absinfo abs_info_p;
    abs_info_p.value = 0;
    abs_info_p.minimum = 0;
    abs_info_p.maximum = BOOGIEBOARD_MAX_P;
    abs_info_p.fuzz = 0;
    abs_info_p.flat = 0;
    abs_info_p.resolution = 1;

    libevdev_enable_event_type(dev, EV_ABS);
    libevdev_enable_event_code(dev, EV_ABS, ABS_PRESSURE, &abs_info_p);
    libevdev_enable_event_code(dev, EV_ABS, ABS_X, &abs_info_x);
    libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &abs_info_y);

    libevdev_enable_property(dev, INPUT_PROP_POINTER);
}

void boogieboard_process_payload(struct libevdev_uinput* uidev, unsigned char* data) {
    int xpos = data[1] & 0xff;
    xpos += (data[2] & 0xff) << 8;
    int ypos = data[3] & 0xff;
    ypos += (data[4] & 0xff) << 8;
    int pressure = data[5] & 0xff;
    pressure += (data[6] & 0xff) << 8;
    int flags = data[7];
    int touch = (flags & BOOGIEBOARD_STYLUS_TOUCH_FLAG) == BOOGIEBOARD_STYLUS_TOUCH_FLAG;
    int hover = (flags & BOOGIEBOARD_STYLUS_HOVER_FLAG) == BOOGIEBOARD_STYLUS_HOVER_FLAG;
    int btn = (flags & BOOGIEBOARD_STYLUS_BTN_FLAG) == BOOGIEBOARD_STYLUS_BTN_FLAG;

    libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOUCH, touch);
    libevdev_uinput_write_event(uidev, EV_KEY, BTN_TOOL_PEN, touch || hover);
    libevdev_uinput_write_event(uidev, EV_ABS, ABS_X, xpos);
    libevdev_uinput_write_event(uidev, EV_ABS, ABS_Y, ypos);
    libevdev_uinput_write_event(uidev, EV_ABS, ABS_PRESSURE, pressure);
    libevdev_uinput_write_event(uidev, EV_KEY, BTN_STYLUS, btn);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}
