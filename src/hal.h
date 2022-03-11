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

#ifndef PUSB_HAL_H_
# define PUSB_HAL_H_
# include <dbus/dbus.h>

DBusConnection *pusb_hal_dbus_connect(void);
void pusb_hal_dbus_disconnect(DBusConnection *dbus);
char *pusb_hal_get_string_property(DBusConnection *dbus, const char *udi, const char *name);
int pusb_hal_get_bool_property(DBusConnection *dbus, const char *udi, const char *name, dbus_bool_t *value);
char **pusb_hal_get_string_array_property(DBusConnection *dbus, const char *udi, const char *name, int *n_items);
int pusb_hal_check_property(DBusConnection *dbus, const char *udi, const char *name, const char *value);
char *pusb_hal_find_item(DBusConnection *dbus, ...);
void pusb_hal_free_string_array(char **str_array, int length);

#endif /* !PUSB_HAL_H_ */
