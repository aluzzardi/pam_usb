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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <libhal-storage.h>
#include "conf.h"
#include "log.h"
#include "hal.h"
#include "volume.h"

static int	pusb_volume_mount(t_pusb_options *opts, LibHalVolume **volume,
				  LibHalContext *ctx)
{
  char		command[1024];
  char		tempname[32];
  const char	*devname;
  const char	*udi;

  snprintf(tempname, sizeof(tempname), "pam_usb%d", getpid());
  if (!(devname = libhal_volume_get_device_file(*volume)))
    {
      log_error("Unable to retrieve device filename\n");
      return (0);
    }
  log_debug("Attempting to mount device %s with label %s\n",
	    devname, tempname);
  snprintf(command, sizeof(command), "pmount -s %s %s",
	   devname, tempname);
  log_debug("Executing \"%s\"\n", command);
  if (system(command) != 0)
    {
      log_error("Mount failed\n");
      return (0);
    }
  udi = libhal_volume_get_udi(*volume);
  if (!udi)
    {
      log_error("Unable to retrieve volume UDI\n");
      return (0);
    }
  udi = strdup(udi);
  libhal_volume_free(*volume);
  *volume = libhal_volume_from_udi(ctx, udi);
  free((char *)udi);
  log_debug("Mount succeeded.\n");
  return (1);
}

static LibHalVolume	*pusb_volume_probe(t_pusb_options *opts,
					   LibHalContext *ctx)
{
  LibHalVolume		*volume = NULL;
  int			maxtries = 0;
  int			i;

  log_debug("Searching for volume with uuid %s\n", opts->device.volume_uuid);
  maxtries = ((opts->probe_timeout * 1000000) / 250000);
  for (i = 0; i < maxtries; ++i)
    {
      char	*udi = NULL;

      if (i == 1)
	log_info("Probing volume (this could take a while)...\n");
      udi = pusb_hal_find_item(ctx,
			       "volume.uuid", opts->device.volume_uuid,
			       NULL);
      if (!udi)
	{
	  usleep(250000);
	  continue;
	}
      volume = libhal_volume_from_udi(ctx, udi);
      libhal_free_string(udi);
      if (!libhal_volume_should_ignore(volume))
	return (volume);
      libhal_volume_free(volume);
      usleep(250000);
    }
  return (NULL);
}

LibHalVolume	*pusb_volume_get(t_pusb_options *opts, LibHalContext *ctx)
{
  LibHalVolume	*volume;

  if (!(volume = pusb_volume_probe(opts, ctx)))
    return (NULL);
  log_debug("Found volume %s\n", opts->device.volume_uuid);
  if (libhal_volume_is_mounted(volume))
    {
      log_debug("Volume is already mounted.\n");
      return (volume);
    }
  if (!pusb_volume_mount(opts, &volume, ctx))
    {
      libhal_volume_free(volume);
      return (NULL);
    }
  return (volume);
}

void		pusb_volume_destroy(LibHalVolume *volume)
{
  const char	*mntpoint;

  mntpoint = libhal_volume_get_mount_point(volume);
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
  libhal_volume_free(volume);
}
