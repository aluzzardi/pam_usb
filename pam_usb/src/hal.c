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

#include <string.h>
#include <dbus/dbus.h>
#include <libhal.h>
#include <libhal-storage.h>
#include "conf.h"
#include "log.h"
#include "otp.h"

static DBusConnection	*pusb_hal_dbus_connect(void)
{
  DBusConnection	*dbus = NULL;
  DBusError		error;

  dbus_error_init(&error);
  if (!(dbus = dbus_bus_get(DBUS_BUS_SYSTEM, &error)))
    {
      log_error("Cannot connect to system bus: %s\n",
		error.message);
      dbus_error_free(&error);
      return (NULL);
    }
  return (dbus);
}

static void	pusb_hal_dbus_disconnect(DBusConnection *dbus)
{
  dbus_connection_close(dbus);
  dbus_connection_unref(dbus);
  dbus_shutdown();
}

static LibHalContext	*pusb_hal_init(DBusConnection *dbus)
{
  DBusError		error;
  LibHalContext		*ctx = NULL;

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

static int	pusb_hal_verify_model(LibHalDrive *drive,
				      t_pusb_options *opts)
{
  if (strcmp(libhal_drive_get_vendor(drive),
	     opts->device.vendor) != 0)
    {
      log_error("Vendor mismatch\n");
      return (0);
    }
  if (strcmp(libhal_drive_get_model(drive),
	     opts->device.model) != 0)
    {
      log_error("Model mismatch\n");
      return (0);
    }
  return (1);
}

LibHalDrive	*pusb_hal_find_drive(LibHalContext *ctx,
				     t_pusb_options *opts)
{
  DBusError	error;
  LibHalDrive	*retval = NULL;
  char		**devices;
  int		n_devices;

  dbus_error_init(&error);
  if (!(devices = libhal_manager_find_device_string_match(ctx,
							  "storage.serial",
							  opts->device.serial,
							  &n_devices,
							  &error)))
    {
      log_error("Unable to find device \"%s\": %s\n", opts->device.name,
		error.message);
      dbus_error_free(&error);
      return (NULL);
    }
  if (!n_devices)
    {
      log_error("Device \"%s\" not connected\n", opts->device.name);
      libhal_free_string_array(devices);
      return (NULL);
    }
  log_debug("Device \"%s\" connected (S/N: %s)\n", opts->device.name,
	    opts->device.serial);
  retval = libhal_drive_from_udi(ctx, devices[0]);
  libhal_free_string_array(devices);
  if (!pusb_hal_verify_model(retval, opts))
    {
      libhal_drive_free(retval);
      return (NULL);
    }
  return (retval);
}

int		pusb_hal_device_check(t_pusb_options *opts)
{
  DBusConnection	*dbus;
  LibHalContext		*ctx;
  LibHalDrive		*drive;
  int			retval;

  if (!(dbus = pusb_hal_dbus_connect()))
    return (0);
  if (!(ctx = pusb_hal_init(dbus)))
    return (0);
  drive = pusb_hal_find_drive(ctx, opts);
  if (!drive)
    {
      pusb_hal_dbus_disconnect(dbus);
      libhal_ctx_free(ctx);
      return (0);
    }
  retval = pusb_otp_check(opts, ctx, drive);
  libhal_drive_free(drive);
  pusb_hal_dbus_disconnect(dbus);
  libhal_ctx_free(ctx);
  return (retval);
}
