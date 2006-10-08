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

#include <unistd.h>
#include <string.h>
#include <libhal-storage.h>
#include "conf.h"
#include "hal.h"
#include "log.h"
#include "otp.h"

LibHalDrive	*pusb_device_get_storage(t_pusb_options *opts, LibHalContext *ctx,
					 const char *udi)
{
  char		*phy_udi = NULL;
  char		*storage_udi = NULL;
  int		maxloop = 0;
  LibHalDrive	*drive = NULL;

  log_debug("Waiting for device to come up...\n");
  while (!(phy_udi = pusb_hal_find_item(ctx,
					"info.parent", udi,
					"info.bus", "usb",
					NULL)))
    usleep(250000);
  maxloop = ((opts->probe_timeout * 1000000) / 250000);
  while (maxloop > 0 &&
	 (!(storage_udi = pusb_hal_find_item(ctx,
					     "storage.physical_device", phy_udi,
					     "info.category", "storage",
					     NULL)) || strstr(storage_udi, "temp")))
    {
      if (storage_udi)
	libhal_free_string(storage_udi);
      --maxloop;
      usleep(250000);
    }
  libhal_free_string(phy_udi);
  if (storage_udi)
    {
      drive = libhal_drive_from_udi(ctx, storage_udi);
      libhal_free_string(storage_udi);
    }
  return (drive);
}

int			pusb_device_check(t_pusb_options *opts)
{
  DBusConnection	*dbus = NULL;
  LibHalContext		*ctx = NULL;
  LibHalDrive		*drive = NULL;
  char			*udi = NULL;
  int			retval = 0;

  if (!(dbus = pusb_hal_dbus_connect()))
    return (0);
  if (!(ctx = pusb_hal_init(dbus)))
    {
      pusb_hal_dbus_disconnect(dbus);
      return (0);
    }

  udi = pusb_hal_find_item(ctx,
			   "usb_device.serial", opts->device.serial,
			   "usb_device.vendor", opts->device.vendor,
			   "info.product", opts->device.model,
			   NULL);
  if (!udi)
    {
      pusb_hal_dbus_disconnect(dbus);
      libhal_ctx_free(ctx);
      return (0);
    }
  log_debug("Valid device %s\n", udi);
  if (!opts->try_otp && !opts->enforce_otp)
    retval = 1;
  else
    {
      if (!(drive = pusb_device_get_storage(opts, ctx, udi)))
	retval = !opts->enforce_otp;
      else
	retval = pusb_otp_check(opts, ctx, drive);
    }
  libhal_free_string(udi);
  pusb_hal_dbus_disconnect(dbus);
  libhal_ctx_free(ctx);
  return (retval);
}
