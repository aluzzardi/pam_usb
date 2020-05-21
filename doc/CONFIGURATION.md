Configuration file reference
============================

The configuration file is formatted in XML and subdivided in 4 sections:

* Default options, shared among every device, user and service
* Devices declaration and settings
* Users declaration and settings
* Services declaration and settings

The syntax is the following:

```xml
<configuration>
    <defaults>
        <!-- default options -->
    </defaults>

    <devices>
        <!-- devices definitions -->
    </devices>

    <users>
        <!-- users definitions -->
    </users>

    <services>
        <!-- services definitions -->
    </services>
</configuration>
```

----------

## Options

|          Name          |  Type   |       Default       |                         Description                          |
|------------------------|---------|---------------------|--------------------------------------------------------------|
| `enable`               | Boolean | `true`              | Enable pam_usb                                               |
| `debug`                | Boolean | `false`             | Enable debug messages                                        |
| `quiet`                | Boolean | `false`             | Quiet mode                                                   |
| `color_log`            | Boolean | `true`              | Enable colored output                                        |
| `one_time_pad`         | Boolean | `true`              | Enable the use of one time device-associated pad files       |
| `deny_remote`          | Boolean | `true`              | Deny access from remote host (SSH)                           |
| `probe_timeout`        | Time    | `10s`               | Time to wait for the volume to be detected                   |
| `pad_expiration`       | Time    | `1h`                | Time between pad file regeneration                           |
| `hostname`             | String  | Computer's hostname | Must be unique accross computers using the same device       |
| `system_pad_directory` | String  | `.pamusb`           | Relative path to the user's home used to store one time pads |
| `device_pad_directory` | String  | `.pamusb`           | Relative path to the device used to store one time pad files |

### Example:

```xml
<configuration>
    <defaults>
        <!-- Disable colored output by default -->
        <option name="color_log">false</option>
        <!-- Enable debug output -->
        <option name="debug">true</option>
    </defaults>

    <users>
        <user id="root">
            <!-- Enable colored output for user "root" -->
            <option name="color_log">true</option>
        </user>

        <user id="scox">
             <!-- Disable debug output for user "scox" -->
             <option name="debug">false</option>
        </user>
    </users>

    <devices>
        <device id="mydevice">
            <!-- Wait 15 seconds instead of the default 10 seconds for "mydevice" to be detected -->
            <option name="probe_timeout">15</option>
        </device>
    </devices>

    <services>
        <service id="su">
            <!-- Disable pam_usb for "su" ("su" will ask for a password as usual) -->
            <option name="enable">false<option>
       </service>
    </services>
</configuration>
```

----------

## Devices


|     Name      |   Type    |                  Description                   |        Example         |
|---------------|-----------|------------------------------------------------|------------------------|
| `id`          | Attribute | Arbitrary device name                          | `MyDevice`             |
| `vendor`      | Element   | Device's vendor name                           | `SanDisk Corp.`        |
| `model`       | Element   | Device's model name                            | `Cruzer Titanium`      |
| `serial`      | Element   | Serial number of the device                    | `SNDKXXXXXXXXXXXXXXXX` |
| `volume_uuid` | Element   | UUID of the device's volume used to store pads | `6F6B-42FC`            |

### Example:

```xml
<device id="MyDevice">
    <vendor>SanDisk Corp.</vendor>
    <model>Cruzer Titanium</model>
    <serial>SNDKXXXXXXXXXXXXXXXX</serial>
    <volume_uuid>6F6B-42FC</volume_uuid>
</device>
```

----------

## Users

|   Name   |   Type    |                Description                |  Example   |
|----------|-----------|-------------------------------------------|------------|
| `id`     | Attribute | Login of the user                         | `root`     |
| `device` | Attribute | `id` of the device associated to the user | `MyDevice` |
| `agent`  | Element   | Agent commands, for use with pamusb-agent |            |

### Agent


| Name  |   Type    |                                             Description                                             |
|-------|-----------|-----------------------------------------------------------------------------------------------------|
| `env` | Attribute | An environment variable for the command. For multiple environment variables use multiple `env` tags |
| `cmd` | Attribute | Agent command, associated with `env` tags in the same agent element                                 |

### Example:

```xml
<user id="scox">
    <device>MyDevice</device>

    <!-- When the user "scox" removes the usb device, lock the screen and pause
    beep-media-player -->
    <agent event="lock">
		<env>DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus</env>
		<env>HOME=/home/scox</env>
		<cmd>gnome-screensaver-command --lock</cmd>
	</agent>
    <agent event="lock">
		<cmd>beep-media-player --pause</cmd>
	</agent>

    <!-- Resume operations when the usb device is plugged back and authenticated -->
    <agent event="unlock">
		<env>DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus</env>
		<env>HOME=/home/scox</env>
		<cmd>gnome-screensaver-command --deactivate</cmd>
	</agent>
    <agent event="unlock">
		<cmd>beep-media-player --play</cmd>
	</agent>
</user>
```

----------

## Services

| Name |   Type    |     Description     | Example |
|------|-----------|---------------------|---------|
| `id` | Attribute | Name of the service | `su`    |

### Example:

```xml
<service id="su">
    <!--
       Here you can put service specific options such as "enable", "debug" etc.
       See the options section of this document.
    -->
</service>
```

----------

Location of the configuration file
----------------------------------

By default, `pam_usb.so` and its tools will look for the configuration file at `/etc/security/pam_usb.conf`.

If you want to use a different location, you will have to use the `-c` flag.

```
# /etc/pam.d/system-auth
auth    sufficient      pam_usb.so -c /some/other/path.conf
auth    required        pam_unix.so nullok_secure
```

You will also have to use the `-c` option when calling pam_usb's tools.

```
pamusb-agent -c /some/other/path.conf
```

Example configuration
----------------------------------

**NOTE**: For detailed information, rely on repository wiki pages.

* **1)** Insert an USB block device
* **2)** Add necessary user configuration into `/etc/security/pam_usb.conf` by running:

```
sudo pamusb-conf --add-user=<username>
```

where `<username>` is a valid Unix user name.

* **3)** Add necessary device configuration into `/etc/security/pam_usb.conf` by running:

```
sudo pamusb-conf --add-device=<devicename>
```

where `<devicename>` is a recognizable name for your device. This value is only used internally in the configuration file as device `id` value.

* **4)** Tweak `/etc/security/pam_usb.conf` manually as desired. Link devices and users, etc.

**NOTE**: If you don't want to use one time pad files, consider setting `one_time_pad` option to `false`. Pad file use defaults to `true`.

If you use one time pads, you need to do the following:

* **5)** Manually mount USB block device partition. You need write access to the mounted partition.

* **6)** Run `/usr/bin/pamusb-check --debug --service=pamusb-agent <username>`

where `<username>` is associated with the USB block device.

By default, this command creates directory `$HOME/.pamusb/` with a protected device-associated `.pad` file. If you format the device, you must
delete `$HOME/.pamusb/<devicename>.pad` file. The created `.pad` file can't be used with a new partition UUIDs for the same or any USB block device.

* **7)** Unmount the USB block device.
* **8)** Add proper PAM configuration into `/etc/pam.d/system-auth` as described above. For testing purposes, it's highly recommended to start with `sufficient` PAM option before possibly moving to `required` or `requisite` option since you can bypass faulty `pam_usb` configurations.
* **9)** Test the device/user configuration by running `sudo echo "pam_usb test"`. The USB block device must be attached (mount not required) and the user must have proper configuration in `/etc/security/pam_usb.conf` file.
