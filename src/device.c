/* -*- mode: c; c-file-style: "bsd"; indent-tabs-mode: t; -*- */

/*
 * Copyright (c) 2016 Luka Novsak <lnovsak@gmail.com>
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <udisks/udisks.h>

#include "conf.h"
#include "log.h"
#include "pad.h"
#include "device.h"

static int pusb_device_connected(t_pusb_options *opts, UDisksClient *udisks)
{
	GDBusObjectManager	*manager = udisks_client_get_object_manager(udisks);
	GList			*objects = g_dbus_object_manager_get_objects(manager);
	int			retval = 0;
	int			i;
	UDisksObject		*object = NULL;
	UDisksDrive		*drive = NULL;

	manager = udisks_client_get_object_manager(udisks);
	objects = g_dbus_object_manager_get_objects(manager);

	log_debug("Searching for \"%s\" in the hardware database...\n",
			opts->device.name);

	for (i = 0; i < g_list_length(objects); ++i)
	{
		object = UDISKS_OBJECT(g_list_nth(objects, i)->data);
		if (udisks_object_peek_drive(object))
		{
			drive = udisks_object_get_drive(object);
			retval = strcmp(udisks_drive_get_serial(drive), opts->device.serial) == 0 &&
					strcmp(udisks_drive_get_vendor(drive), opts->device.vendor) == 0 &&
					strcmp(udisks_drive_get_model(drive), opts->device.model) == 0;
			g_object_unref(drive);
			if (retval)
				break;
		}
	}

	if (retval)
		log_info("Device \"%s\" is connected (good).\n",
				opts->device.name);
	else
		log_error("Device \"%s\" is not connected (bad).\n",
				opts->device.name);

	g_list_foreach (objects, (GFunc) g_object_unref, NULL);
	g_list_free (objects);

	return (retval);
}

int pusb_device_check(t_pusb_options *opts, const char *user)
{
	UDisksClient	*udisks = NULL;
	int		retval = 0;

	udisks = udisks_client_new_sync(NULL, NULL);

	if (!pusb_device_connected(opts, udisks))
	{
		g_object_unref(udisks);
		return (0);
	}

	if (opts->one_time_pad)
	{
		log_info("Performing one time pad verification...\n");
		retval = pusb_pad_check(opts, udisks, user);
	}
	else
	{
		log_debug("One time pad is disabled, no more verifications to do.\n");
		retval = 1;
	}

	g_object_unref(udisks);
	return (retval);
}
