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
#include "conf.h"
#include "log.h"

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

static int		pusb_hal_verify_model(LibHalContext *ctx,
						     t_pusb_options *opts,
						     const char *udi)
{
  DBusError		error;
  char			*data;
  int			i;
  struct s_opt_list	check_list[] = {
    { "usb_device.vendor", opts->device.vendor },
    { "info.product", opts->device.model },
    { NULL, NULL }
  };

  log_debug("Verifying model...\n");
  dbus_error_init(&error);
  for (i = 0; check_list[i].name; ++i)
    {
      data = libhal_device_get_property_string(ctx, udi,
					       check_list[i].name,
					       &error);
      if (!data)
	{
	  log_error("Cannot retrieve device %s: %s\n",
		    check_list[i].name,
		    error.message);
	  dbus_error_free(&error);
	  return (0);
	}
      if (strcmp(data, check_list[i].value) != 0)
	{
	  log_error("[KO]\t%s -> %s\n", check_list[i].name, data);
	  libhal_free_string(data);
	  return (0);
	}
      log_debug("[OK]\t%s -> %s \n", check_list[i].name, data);
      libhal_free_string(data);
    }
  return (1);
}

static int	pusb_hal_find_device(LibHalContext *ctx,
				     t_pusb_options *opts)
{
  DBusError	error;
  char		**devices;
  int		n_devices;
  int		retval = 0;

  dbus_error_init(&error);
  if (!(devices = libhal_manager_find_device_string_match(ctx,
							  "usb_device.serial",
							  opts->device.serial,
							  &n_devices,
							  &error)))
    {
      log_error("Unable to find device \"%s\": %s\n", opts->device.name,
		error.message);
      dbus_error_free(&error);
      return (0);
    }
  if (n_devices > 0)
    {
      log_debug("Device \"%s\" connected (S/N: %s)\n", opts->device.name,
		opts->device.serial);
      retval = pusb_hal_verify_model(ctx, opts, devices[0]);
    }
  else
    log_error("Device \"%s\" not connected\n", opts->device.name);
  libhal_free_string_array(devices);
  return (retval);
}

int		pusb_hal_device_check(t_pusb_options *opts)
{
  DBusConnection	*dbus;
  LibHalContext		*ctx;
  int			retval;

  if (!(dbus = pusb_hal_dbus_connect()))
    return (0);
  if (!(ctx = pusb_hal_init(dbus)))
    return (0);
  retval = pusb_hal_find_device(ctx, opts);
  pusb_hal_dbus_disconnect(dbus);
  libhal_ctx_free(ctx);
  return (retval);
}
