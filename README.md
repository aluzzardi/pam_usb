pam\_usb provides hardware authentication for Linux using ordinary USB Flash Drives.

It works with any application supporting PAM, such as _su_, any login manager (_GDM_, _KDM_), etc. The pam\_usb package contains:

* A PAM Module
    * Password-less authentication. Use your USB stick for authentication, don't type passwords anymore.
    * Device auto probing.You don't need to mount the device, or even to configure the device location (_sda1_, _sdb1_, etc). pam\_usb.so will automatically locate the device using _HAL_ and access its data by itself.
    * Two-factor authentication. Achieve greater security by requiring both the USB stick and the password to authenticate the user.
    * Non-intrusive. pam\_usb doesn't require any modifications of the USB storage device to work (no additional partitions required).
    * USB Serial number, model and vendor verification.
    * Support for **One Time Pads** authentication.
    * You can use the same device accross multiple machines.
    * Support for all kind of removable devices (SD, MMC, etc).
* Several tools
    * **pamusb-agent**: trigger actions (such as locking the screen) upon device authentication and removal.
    * **pamusb-conf**: configuration helper.
    * **pamusb-check**: integrate pam\_usb's authentication engine within your scripts or applications.
