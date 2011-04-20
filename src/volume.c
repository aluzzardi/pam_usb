/*
 * Copyright (c) 2003-2007 Andrea Luzzardi <scox@sig11.org>
 *
 * This file is part of the pam_usb project. pam_usb is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * pam_usb is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mount.h>
#include "conf.h"
#include "log.h"
#include "hal.h"
#include "volume.h"

static int pusb_volume_mount(t_pusb_options *opts, char *udi,
		DBusConnection *dbus)
{
	char		command[1024];
	char		tempname[32];
	const char	*devname;

	snprintf(tempname, sizeof(tempname), "pam_usb%d", getpid());
	if (!(devname = pusb_hal_get_string_property(dbus, udi, "DeviceFile")))
	{
		log_error("Unable to retrieve device filename\n");
		return (0);
	}
	log_debug("Attempting to mount device %s with label %s\n",
			devname, tempname);
	snprintf(command, sizeof(command), "pmount -A -s %s %s",
			 devname, tempname);
	log_debug("Executing \"%s\"\n", command);
	if (system(command) != 0)
	{
		log_error("Mount failed\n");
		return (0);
	}

	log_debug("Mount succeeded.\n");
	return (1);
}

static char *pusb_volume_mount_path(t_pusb_options *opts, char *udi, DBusConnection* dbus)
{
	dbus_bool_t is_mounted;
	if (!pusb_hal_get_bool_property(dbus, udi, "DeviceIsMounted", &is_mounted))
	{
		return (NULL);
	}
	if (is_mounted != TRUE)
	{
		log_debug("Device %s is not mounted\n", udi);
		return (NULL);
	}

	int n_mount;
	char **mount_pathes = pusb_hal_get_string_array_property(dbus, udi, "DeviceMountPaths", &n_mount);
	if (!mount_pathes)
	{
		log_debug("Failed to retrieve device %s mount path\n", udi);
		return (NULL);
	}
	if (n_mount > 1)
	{
		log_debug("Device %s is mounted more than once\n", udi);
	}
	char *mount_path = strdup(mount_pathes[0]);
	pusb_hal_free_string_array(mount_pathes, n_mount);
	log_debug("Device %s is mounted on %s\n", udi, mount_path);
	return (mount_path);
}

static char	*pusb_volume_probe(t_pusb_options *opts,
		DBusConnection *dbus)
{
	int				maxtries = 0;
	int				i;

	if (!*(opts->device.volume_uuid))
	{
		log_debug("No UUID configured for device\n");
		return (NULL);
	}
	log_debug("Searching for volume with uuid %s\n", opts->device.volume_uuid);
	maxtries = ((opts->probe_timeout * 1000000) / 250000);
	for (i = 0; i < maxtries; ++i)
	{
		char	*udi = NULL;

		if (i == 1)
			log_info("Probing volume (this could take a while)...\n");
		udi = pusb_hal_find_item(dbus,
				"IdUuid", opts->device.volume_uuid,
				NULL);
		if (!udi)
		{
			usleep(250000);
			continue;
		}
		return (udi);
	}
	return (NULL);
}

char *pusb_volume_get(t_pusb_options *opts, DBusConnection *dbus)
{
	char	*volume_udi;
	char	*mount_point;

	if (!(volume_udi = pusb_volume_probe(opts, dbus)))
		return (NULL);
	log_debug("Found volume %s\n", opts->device.volume_uuid);
	mount_point = pusb_volume_mount_path(opts, volume_udi, dbus);
	if (mount_point)
	{
		log_debug("Volume is already mounted.\n");
		return (mount_point);
	}
	if (!pusb_volume_mount(opts, volume_udi, dbus))
	{
		free(volume_udi);
		return (NULL);
	}
	mount_point = pusb_volume_mount_path(opts, volume_udi, dbus);
	if (!mount_point)
	{
		log_error("Unable to retrieve %s mount point\n", volume_udi);
		return (NULL);
	}
	return (mount_point);
}

void pusb_volume_destroy(char *mntpoint)
{
	if (mntpoint && strstr(mntpoint, "pam_usb"))
	{
		char	command[1024];

		log_debug("Attempting to umount %s\n",
				mntpoint);
		snprintf(command, sizeof(command), "pumount %s", mntpoint);
		log_debug("Executing \"%s\"\n", command);
		if (!system(command))
			log_debug("Umount succeeded.\n");
		else
			log_error("Unable to umount %s\n", mntpoint);
	}
	free(mntpoint);
}
