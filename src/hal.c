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
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include "mem.h"
#include "log.h"
#include "hal.h"

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

void pusb_hal_free_string_array(char **str_array, int length)
{
	int i;

	if (str_array == NULL)
      return ;

	for (i = 0; i < length; ++i)
	  xfree(str_array[i]);
	xfree(str_array);
}

char **pusb_hal_get_string_array_from_iter(DBusMessageIter *iter, int *num_elements)
{
	int count;
	char **buffer;

	count = 0;
	buffer = (char **)xmalloc(sizeof(char *) * 8);

	buffer[0] = NULL;
	while (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_STRING ||
		   dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_OBJECT_PATH)
	{
		const char *value;
		
		if ((count % 8) == 0 && count != 0) {
			buffer = xrealloc(buffer, sizeof (char *) * (count + 8));
		}
		
		dbus_message_iter_get_basic(iter, &value);
		buffer[count] = xstrdup(value);

		dbus_message_iter_next(iter);
		count++;
	}

	if (num_elements != NULL)
		*num_elements = count;
	return buffer;
}



DBusMessage *pusb_hal_get_raw_property(DBusConnection *dbus,
									   const char *udi,
									   const char *name)
{
	DBusMessage *message;
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusError error;
	char *iface = "org.freedesktop.UDisks.Device";

	message = dbus_message_new_method_call("org.freedesktop.UDisks", udi,
										   "org.freedesktop.DBus.Properties",
										   "Get");
	if (message == NULL) {
		log_error("Could not allocate D-BUS message\n");
		return (NULL);
	}
	dbus_message_iter_init_append(message, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &name);

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus,
													  message, -1,
													  &error);
	dbus_message_unref(message);
	if (dbus_error_is_set(&error)) {
		log_error("Error communicating with D-BUS\n");
		return (NULL);
	}
	dbus_error_free(&error);
	return (reply);
}

char *pusb_hal_get_string_property(DBusConnection *dbus,
								   const char *udi,
								   const char *name)
{
	DBusMessage *reply;
	DBusMessageIter reply_iter;
	char *data;
	char *dbus_str;

	reply = pusb_hal_get_raw_property(dbus, udi, name);
	if (reply == NULL) {
		return (NULL);
	}

	dbus_message_iter_init(reply, &reply_iter);

	if (dbus_message_iter_get_arg_type(&reply_iter) != DBUS_TYPE_VARIANT)
	{
		dbus_message_unref(reply);
		return (NULL);
	}

	DBusMessageIter subiter;
	dbus_message_iter_recurse(&reply_iter, &subiter);
	dbus_message_iter_get_basic(&subiter, &dbus_str);
	if (dbus_str != NULL)
		data = xstrdup(dbus_str);
	dbus_message_unref(reply);
	return (data);
}

char **pusb_hal_get_string_array_property(DBusConnection *dbus,
										  const char *udi,
										  const char *name,
										  int *n_items)
{
	DBusMessage *reply;
	DBusMessageIter reply_iter;
	char **items;

	reply = pusb_hal_get_raw_property(dbus, udi, name);
	if (reply == NULL) {
		return (NULL);
	}

	dbus_message_iter_init(reply, &reply_iter);

	if (dbus_message_iter_get_arg_type(&reply_iter) != DBUS_TYPE_VARIANT)
	{
		dbus_message_unref(reply);
		return (NULL);
	}

	DBusMessageIter subiter, subsubiter;
	dbus_message_iter_recurse(&reply_iter, &subiter);
	dbus_message_iter_recurse(&subiter, &subsubiter);
	items = pusb_hal_get_string_array_from_iter(&subsubiter, n_items);
	dbus_message_unref(reply);
	if (!*n_items)
	{
		pusb_hal_free_string_array(items, *n_items);
		return (NULL);
	}
	return (items);
}



int pusb_hal_get_bool_property(DBusConnection *dbus,
							   const char *udi,
							   const char *name,
							   dbus_bool_t *value)
{
	DBusMessage *reply;
	DBusMessageIter reply_iter;

	reply = pusb_hal_get_raw_property(dbus, udi, name);
	if (reply == NULL) {
		return (0);
	}

	dbus_message_iter_init(reply, &reply_iter);

	if (dbus_message_iter_get_arg_type(&reply_iter) !=
		   DBUS_TYPE_VARIANT)
	{
		dbus_message_unref(reply);
		return (0);
	}

	DBusMessageIter subiter;
	dbus_message_iter_recurse(&reply_iter, &subiter);
	dbus_message_iter_get_basic(&subiter, value);
	dbus_message_unref(reply);
	return (1);
}

int pusb_hal_check_property(DBusConnection *dbus,
		const char *udi,
		const char *name,
		const char *value)
{
	char	*data;
	int		retval;

	data = pusb_hal_get_string_property(dbus, udi, name);
	if (!data)
		return (0);
	retval = (strcmp(data, value) == 0);
	xfree(data);
	return (retval);
}

char **pusb_hal_find_all_items(DBusConnection *dbus, int *count)
{
	DBusError	error;
	DBusMessage *message;
	DBusMessage *reply;
	DBusMessageIter iter_array, reply_iter;
	char		**devices;
	int			n_devices;

	*count = 0;
	message = dbus_message_new_method_call("org.freedesktop.UDisks",
										   "/org/freedesktop/UDisks",
										   "org.freedesktop.UDisks",
										   "EnumerateDevices");
	if (message == NULL)
	{
		log_error("Couldn't allocate D-BUS message\n");
		return (NULL);
	}
	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(dbus,
													  message, -1,
													  &error);
	dbus_message_unref(message);
	if (dbus_error_is_set(&error)) {
		log_error("Error communicating with D-BUS\n");
		return (NULL);
	}
	if (reply == NULL) {
		return (NULL);
	}
	dbus_message_iter_init(reply, &reply_iter);
	if (dbus_message_iter_get_arg_type(&reply_iter) != DBUS_TYPE_ARRAY) {
		log_error("Malformed D-BUS reply");
		return (NULL);
	}
	dbus_message_iter_recurse(&reply_iter, &iter_array);
	devices = pusb_hal_get_string_array_from_iter(&iter_array, &n_devices);
	dbus_message_unref(reply);
	if (!n_devices)
	{
		pusb_hal_free_string_array(devices, n_devices);
		return (NULL);
	}
	*count = n_devices;
	return (devices);
}

char *pusb_hal_find_item(DBusConnection *dbus,
		...)
{
	char	**devices;
	int		n_devices;
	char	*udi = NULL;
	va_list	ap;
	int		i;

	devices = pusb_hal_find_all_items(dbus, &n_devices);
	if (!devices)
		return (NULL);
	if (!n_devices)
		return (NULL);

	for (i = 0; i < n_devices; ++i)
	{
		char	*key = NULL;
		int		match = 1;

		va_start(ap, dbus);
		while ((key = va_arg(ap, char *)))
		{
			char	*value = NULL;

			value = va_arg(ap, char *);
			if (!value || *value == 0x0)
				continue ;
			if (!pusb_hal_check_property(dbus, devices[i],
						key, value))
			{
				match = 0;
				break;
			}
		}
		if (match)
		{
			udi = xstrdup(devices[i]);
			break;
		}
		va_end(ap);
	}
	pusb_hal_free_string_array(devices, n_devices);
	return (udi);
}
