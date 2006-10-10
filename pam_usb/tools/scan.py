#!/usr/bin/env python2.4

import dbus

bus = dbus.SystemBus()
halService = bus.get_object('org.freedesktop.Hal',
                            '/org/freedesktop/Hal/Manager')
halManager = dbus.Interface(halService, 'org.freedesktop.Hal.Manager')

def getStorageDevice(udi):
    for child in halManager.FindDeviceByCapability('storage'):
        deviceObj = bus.get_object('org.freedesktop.Hal',
                                   child)
        deviceProperties = deviceObj.GetAllProperties(
            dbus_interface = 'org.freedesktop.Hal.Device')
        if deviceProperties['storage.physical_device'] == udi + '_if0':
            return child
    return None

def showVolumes(udi):
    for volume in halManager.FindDeviceByCapability('volume'):
        deviceObj = bus.get_object('org.freedesktop.Hal',
                                   volume)
        deviceProperties = deviceObj.GetAllProperties(
            dbus_interface = 'org.freedesktop.Hal.Device')
        if deviceProperties['block.storage_device'] != udi:
            continue
        print '%s\t\t%s' % (deviceProperties['block.device'],
                           deviceProperties['volume.uuid'])

def showProperties(udi):
    storage = getStorageDevice(udi)
    if storage is None:
        return
    print
    deviceObj = bus.get_object('org.freedesktop.Hal',
                               udi)
    deviceProperties = deviceObj.GetAllProperties(
        dbus_interface = 'org.freedesktop.Hal.Device')
    print 'Device %s' % udi
    print 'Serial Number: %s' % deviceProperties['usb_device.serial']
    print
    print 'Volume\t\t\tUUID'
    showVolumes(storage)

bus = dbus.SystemBus()
halService = bus.get_object('org.freedesktop.Hal',
                            '/org/freedesktop/Hal/Manager')
halManager = dbus.Interface(halService, 'org.freedesktop.Hal.Manager')

print 'Scanning USB devices...'
for udi in halManager.FindDeviceStringMatch('info.bus', 'usb_device'):
    deviceObj = bus.get_object('org.freedesktop.Hal',
                               udi)
    showProperties(udi)
