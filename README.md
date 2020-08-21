pam\_usb
========

pam\_usb provides hardware authentication for Linux using ordinary USB Flash Drives.

It works with any application supporting PAM, such as _su_ and login managers (_GDM_, _KDM_).

Features
--------

* `Password-less authentication.` Use your USB stick for authentication, don't type passwords anymore.
* `Device auto probing.` You don't need to mount the device, or even to configure the device location (_sda1_, _sdb1_, etc). pam\_usb.so will automatically locate the device using `UDisks` and access its data by itself.
* `Two-factor authentication.` Achieve greater security by requiring both the USB stick and the password to authenticate the user.
* `Non-intrusive.` pam\_usb doesn't require any modifications of the USB storage device to work (no additional partitions required).
* USB Serial number, model and vendor verification.
* Support for **One Time Pads** authentication.
* You can use the same device across multiple machines.
* Support for all kind of removable devices (SD, MMC, etc).

Tools
-----
* `pamusb-agent`: trigger actions (such as locking the screen) upon device authentication and removal.
* `pamusb-conf`: configuration helper.
* `pamusb-check`: integrate pam\_usb's authentication engine within your scripts or applications.

Getting Started
----------------
* [Install](https://github.com/aluzzardi/pam_usb/wiki/Install) pam_usb on your system
* Read the [Getting Started Guide](https://github.com/aluzzardi/pam_usb/wiki/Getting-Started)
* Have a look at the [Configuration File Reference](https://github.com/aluzzardi/pam_usb/wiki/Configuration)
* Problem? See the [Troubleshooting Guide](https://github.com/aluzzardi/pam_usb/wiki/Troubleshooting)

History of this PAM module
----------------
This software was initially created by Andrea Luzzardi (https://github.com/aluzzardi/pam_usb/) but is
unmaintained for quite some years by now. While the years passed the community continued to work on it
but there was never a centralized place merging all those improvement. The goal of this repo is to provide
exactly that, resulting in an up-to-date version that will also be released in a (yet to be setup) PPA.

This repo is mainly based on community improvements from Pekka Helenius (https://github.com/Fincer/pam_usb),
"IGP" (https://github.com/IGP/pam_usb)", "McDope" (https://github.com/mcdope/pam_usb), and some others. 
See the commit history for details.

The changes since the last "official" release are documented in the file `changelog-from-v0.5.0`.
