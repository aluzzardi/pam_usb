pam\_usb
========

pam\_usb provides hardware authentication for Linux using ordinary USB Flash Drives.

It works with any application supporting PAM, such as _su_ and login managers (_GDM_, _KDM_).

Features
--------

* `Password-less authentication.` Use your USB stick for authentication, don't type passwords anymore.
* `Device auto probing.` You don't need to mount the device, or even to configure the device location (_sda1_, _sdb1_, etc). pam\_usb.so will automatically locate the device using _HAL_ and access its data by itself.
* `Two-factor authentication.` Achieve greater security by requiring both the USB stick and the password to authenticate the user.
* `Non-intrusive.` pam\_usb doesn't require any modifications of the USB storage device to work (no additional partitions required).
* USB Serial number, model and vendor verification.
* Support for **One Time Pads** authentication.
* You can use the same device accross multiple machines.
* Support for all kind of removable devices (SD, MMC, etc).

Tools
-----
* `pamusb-agent`: trigger actions (such as locking the screen) upon device authentication and removal.
* `pamusb-conf`: configuration helper.
* `pamusb-check`: integrate pam\_usb's authentication engine within your scripts or applications.

Installing
==========

pam_usb is included in most Linux distributions out there.

If you happen to run Ubuntu or Debian, run:

    # apt-get install libpam-usb pamusb-tools

Gentoo (You may need to unmask `sys-auth/pam_usb` or use `--automask-write`):

    # emerge pam_usb

Compiling from source
---------------------

Before proceeding, make sure all dependencies are installed: libxml2, pam, udisks and pmount are installed

    $ make
    # make install

Configuring
===========

By default, pam_usb will read its configuration from `/etc/pamusb.conf`

For most operations, you can use ``pamusb-conf`` which will take care of generating the configuration for you.

Setting up Devices and Users
----------------------------

Once you've connected your USB device to the computer, use pamusb-conf to add it to the configuration file:

    # pamusb-conf --add-device MyDevice
    Please select the device you wish to add.
    * Using "SanDisk Corp. Cruzer Titanium (SNDKXXXXXXXXXXXXXXXX)" (only option)
    Which volume would you like to use for storing data ?
    * Using "/dev/sda1 (UUID: <6F6B-42FC>)" (only option)
    Name            : MyDevice
    Vendor          : SanDisk Corp.
    Model           : Cruzer Titanium
    Serial          : SNDKXXXXXXXXXXXXXXXX
    Volume UUID     : 6F6B-42FC (/dev/sda1)
    Save to /etc/pamusb.conf ?
    [Y/n] y
    Done.

Note that `MyDevice` can be any arbitrary name you'd like. Also, you can add as many devices as you want.

Next, configure users you want to be able to authenticate with pam_usb:

    # pamusb-conf --add-user root      
    Which device would you like to use for authentication ?
    * Using "MyDevice" (only option)
    User            : root
    Device          : MyDevice
    Save to /etc/pamusb.conf ?
    [Y/n] y
    Done.

Check the configuration
-----------------------

You can run `pamusb-check` anytime to check if everything is correctly worked.
This tool will simulate an authentication request (requires your device to be connected, otherwise it will fail).

    # pamusb-check root
    * Authentication request for user "root" (pamusb-check)
    * Device "MyDevice" is connected (good).
    * Performing one time pad verification...
    * Verification match, updating one time pads...
    * Access granted.


Setting up the PAM module
-------------------------

To add pam_usb into the system authentication process, we need to edit `/etc/pam.d/common-auth`
    
    NOTE: If you are using RedHat or Fedora this file can be known as /etc/pam/system-auth.

Your default PAM common-auth configuration should include the following line:

    auth    required        pam_unix.so nullok_secure

This is a current standard which uses passwords to authenticate a user.

Alter your /etc/pam.d/common-auth configuration to:

    auth    sufficient      pam_usb.so
    auth    required        pam_unix.so nullok_secure

The `suffient` keyword means that if pam_usb allows the authentication, then no password will be asked.
If the authentication fails, then the default password-based authentication will be used as fallback.

If you change it to `required`, it means that *both* the USB flash drive and the password will be required to grant
access to the system.

At this point, you should be able to authenticate with the relevant USB device plugged-in.

    scox $ su
    * pam_usb v.SVN
    * Authentication request for user "root" (su)
    * Device "MyDevice" is connected (good).
    * Performing one time pad verification...
    * Verification match, updating one time pads...
    * Access granted.

Agent
-----

The pam_usb agent (pamusb-agent) allows you to automatically execute commands
upon locking and unlocking events. Those events are generated when you insert or
remove your authentication device.
To configure the commands, you have to edit pam_usb's configuration file
(/etc/pamusb.conf) and add agent entries into your user section.

For instance, you could automatically start your screensaver as soon as you
remove the device, and deactivate it when you plug the device back.

GNOME (gnome-screensaver):

```xml
<user id="scox">
  <device>MyDevice</device>
  <agent event="lock">gnome-screensaver-command --lock</agent>
  <agent event="unlock">gnome-screensaver-command --deactivate</agent>
</user>
```

KDE (kscreensaver):

```xml
<user id="scox">
  <device>MyDevice</device>
  <agent event="lock">dcop kdesktop KScreensaverIface lock</agent>
  <agent event="unlock">dcop kdesktop KScreensaverIface quit</agent>
</user>
```

You can execute more commands by adding extra `<agent>` entries.

    $ pamusb-agent
    pamusb-agent[18329]: pamusb-agent up and running.
    pamusb-agent[18329]: Watching device "MyDevice" for user "scox"
    pamusb-agent[18329]: Device "MyDevice" has been removed, locking down user
    "scox"...
    pamusb-agent[18329]: Running "gnome-screensaver-command --lock"
    pamusb-agent[18329]: Locked.
    pamusb-agent[18329]: Device "MyDevice" has been inserted. Performing
    verification...
    pamusb-agent[18329]: Executing "/usr/bin/pamusb-check --quiet
    --config=/etc/pamusb.conf --service=pamusb-agent scox"
    pamusb-agent[18329]: Authentication succeeded. Unlocking user "scox"...
    pamusb-agent[18329]: Running "gnome-screensaver-command --deactivate"
    pamusb-agent[18329]: Unlocked.

Depending on your desktop environment, you have to add pamusb-agent to the list
of autostarted applications so it will be started automatically.

GNOME:

- Open System -> Preferences -> Sessions
- Select Startup Programs and press Add
- Enter pamusb-agent and press OK
- Press Close

KDE:

    cd ~/.kde/Autostart
    ln -s /usr/bin/pamusb-agent pamusb-agent

Configuration Reference
-----------------------

There are many more options available to fine tune pam_usb.

Check out the [configuration reference](https://github.com/aluzzardi/pam_usb/blob/master/doc/CONFIGURATION.md).

Troubleshooting
===============

Log Analysis
------------

Both pam_usb.so and pamusb-agent use the syslog facility to log authentication
attempts.
This can be useful for GUI-driven applications (for instance GDM) where you
don't get to see console output.
Messages are logged with the AUTH facility, they are usually written to
`/var/log/auth.log` but may vary
depending on the operating system you're using.

    # tail -f /var/log/auth.log
    pamusb-agent[25429]: Device "sandisk" has been inserted. Performing
    verification...
    pamusb-agent[25429]: Executing "/usr/bin/pamusb-check --quiet
    --config=/etc/pamusb.conf --service=pamusb-agent scox"
    pam_usb[25485]: Authentication request for user "scox" (pamusb-agent)
    pam_usb[25485]: Device "sandisk" is connected (good).
    pam_usb[25485]: Access granted.
    pamusb-agent[25429]: Authentication succeeded. Unlocking user "scox"...
    pamusb-agent[25429]: Unlocked.


Enabling debug
--------------

Enabling debug messages may help you find out what's wrong.

To enable them, edit `/etc/pamusb.conf` and set the following option:

```xml
<defaults>
  <option name="debug">true</option>
</defaults>
```
You can enable debug messages only for a specific user, device or service.

```xml
<services>
  <service id="sudo">
    <option name="debug">true</option>
  </service>
</services>
```
