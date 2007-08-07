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

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <dbus/dbus.h>
#include <libhal-storage.h>
#include "log.h"

DBusConnection *pusb_hal_dbus_connect(void)
{
	DBusConnection	*dbus = NULL;
	DBusError		error;

	dbus_error_init(&error);
	if (!(dbus = dbus_bus_get(DBUS_BUS_SYSTEM, &error)))
	{
		/* Workaround for https://bugs.freedesktop.org/show_bug.cgi?id=11876 */
		uid_t			ruid;
		uid_t			euid;

		if (!(euid = geteuid()) && (ruid = getuid()))
		{
			dbus_error_free(&error);
			setreuid(euid, euid);
			dbus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
			setreuid(ruid, euid);
		}
		if (!dbus)
		{
			log_error("Cannot connect to system bus: %s\n",
					error.message);
			dbus_error_free(&error);
			return (NULL);
		}
	}
	return (dbus);
}

void pusb_hal_dbus_disconnect(DBusConnection *dbus)
{
	dbus_connection_unref(dbus);
}

LibHalContext *pusb_hal_init(DBusConnection *dbus)
{
	DBusError		error;
	LibHalContext	*ctx = NULL;

	dbus_error_init(&error);
	if (!(ctx = libhal_ctx_new()))
	{
		log_error("Failed to create a HAL context\n");
		return (NULL);
	}
	if (!libhal_ctx_set_dbus_connection(ctx, dbus))
	{
		log_error("Failed to attach dbus connection to hal\n");
		libhal_ctx_free(ctx);
		return (NULL);
	}
	if (!libhal_ctx_init(ctx, &error))
	{
		log_error("libhal_ctx_init: %s\n", error.name, error.message);
		libhal_ctx_free(ctx);
		return (NULL);
	}
	return (ctx);
}

void pusb_hal_destroy(LibHalContext *ctx)
{
	libhal_ctx_free(ctx);
}

char *pusb_hal_get_property(LibHalContext *ctx,
		const char *udi,
		const char *name)
{
	DBusError	error;
	char		*data;

	dbus_error_init(&error);
	data = libhal_device_get_property_string(ctx, udi,
			name, &error);
	if (!data)
	{
		log_debug("%s\n", error.message);
		dbus_error_free(&error);
		return (NULL);
	}
	return (data);
}

int pusb_hal_check_property(LibHalContext *ctx,
		const char *udi,
		const char *name,
		const char *value)
{
	char	*data;
	int		retval;

	data = pusb_hal_get_property(ctx, udi, name);
	if (!data)
		return (0);
	retval = (strcmp(data, value) == 0);
	libhal_free_string(data);
	return (retval);
}

char **pusb_hal_find_all_items(LibHalContext *ctx,
		const char *property,
		const char *value,
		int *count)

{
	DBusError	error;
	char		**devices;
	int			n_devices;

	dbus_error_init(&error);
	*count = 0;
	devices = libhal_manager_find_device_string_match(ctx,
			property,
			value,
			&n_devices,
			&error);
	if (!devices)
	{
		log_error("Unable to find item \"%s\": %s\n", property,
				error.message);
		dbus_error_free(&error);
		return (NULL);
	}
	if (!n_devices)
	{
		libhal_free_string_array(devices);
		return (NULL);
	}
	*count = n_devices;
	return (devices);
}

char *pusb_hal_find_item(LibHalContext *ctx,
		const char *property,
		const char *value,
		...)
{
	char	**devices;
	int		n_devices;
	char	*udi = NULL;
	va_list	ap;
	int		i;

	devices = pusb_hal_find_all_items(ctx, property, value, &n_devices);
	if (!devices)
		return (NULL);
	if (!n_devices)
		return (NULL);

	for (i = 0; i < n_devices; ++i)
	{
		char	*key = NULL;
		int		match = 1;

		va_start(ap, value);
		while ((key = va_arg(ap, char *)))
		{
			char	*value = NULL;

			value = va_arg(ap, char *);
			if (!value || *value == 0x0)
				continue ;
			if (!pusb_hal_check_property(ctx, devices[i],
						key, value))
			{
				log_debug("%s did match, but property %s didn't (expecting \"%s\")\n",
						property, key, value);
				match = 0;
				break;
			}
		}
		if (match)
		{
			udi = strdup(devices[i]);
			break;
		}
		va_end(ap);
	}
	libhal_free_string_array(devices);
	return (udi);
}
