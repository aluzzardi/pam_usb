/*
 * Copyright (c) 2003-2006 Andrea Luzzardi <scox@sig11.org>
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

#ifndef HAL_H_
# define HAL_H_

DBusConnection *pusb_hal_dbus_connect(void);
void pusb_hal_dbus_disconnect(DBusConnection *dbus);
LibHalContext *pusb_hal_init(DBusConnection *dbus);
void pusb_hal_destroy(LibHalContext *ctx);
char *pusb_hal_get_property(LibHalContext *ctx,
			    const char *udi,
			    const char *name);
int pusb_hal_check_property(LibHalContext *ctx,
			    const char *udi,
			    const char *name,
			    const char *value);
char *pusb_hal_find_item(LibHalContext *ctx,
			 const char *property,
			 const char *value,
			 ...);

#endif /* !HAL_H_ */
