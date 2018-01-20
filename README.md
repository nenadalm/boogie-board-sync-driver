# BoogieBoard Sync Driver

Userspace driver for [BoogieBoard Sync](https://myboogieboard.com/products/sync) tablet.

This driver was rewritten from python (https://github.com/jbedo/boogiesync-tablet) into `C` and currently supports only USB.

## Installation

### [Fedora](https://getfedora.org/)

```shell
$ dnf install libevdev-devel libusbx-devel
$ make
```

## Run

```shell
$ ./bin/usb
```

