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
#include "volume.h"

static int		pusb_volume_mount(t_pusb_options *opts, LibHalVolume **volume,
					  LibHalContext *ctx)
{
  char		command[1024];
  char		tempname[32];
  const char	*devname;
  const char	*fs;

  snprintf(tempname, sizeof(tempname), "pam_usb%d", getpid());
  if (!(fs = libhal_volume_get_fstype(*volume)))
    {
      log_error("Unable to retrieve filesystem type\n");
      return (0);
    }
  if (!(devname = libhal_volume_get_device_file(*volume)))
    {
      log_error("Unable to retrieve device filename\n");
      return (0);
    }
  log_debug("Attempting to mount device %s with label %s (fs: %s)\n",
	    devname, tempname, fs);
  snprintf(command, sizeof(command), "pmount -s -t %s %s %s",
	   fs, devname, tempname);
  log_debug("Executing \"%s\"\n", command);
  if (system(command) != 0)
    {
      log_error("Mount failed\n");
      return (0);
    }
  else
    {
      const char	*udi;

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
    }
  log_debug("Mount succeeded.\n");
  return (libhal_volume_is_mounted(*volume));
}

static int	__pusb_volume_find(t_pusb_options *opts, LibHalContext *ctx,
				   LibHalDrive *drive, LibHalVolume **out)
{
  char		**volumes;
  int		n_volumes = 0;
  int		i;

  *out = NULL;
  volumes = libhal_drive_find_all_volumes(ctx, drive, &n_volumes);
  if (!n_volumes)
    {
      libhal_free_string_array(volumes);
      log_debug("No volumes found\n");
      return (1);
    }
  for (i = 0; i < n_volumes; ++i)
    {
      LibHalVolume	*volume;

      volume = libhal_volume_from_udi(ctx,
				      volumes[i]);
      if (!volume)
	continue;
      if (libhal_volume_should_ignore(volume))
	{
	  libhal_volume_free(volume);
	  continue;
	}
      *out = volume;
      libhal_free_string_array(volumes);
      if (libhal_volume_is_mounted(volume))
	{
	  log_debug("Volume is already mounted\n");
	  return (1);
	}
      else
	{
	  if (pusb_volume_mount(opts, &volume, ctx))
	    return (1);
	  return (0);
	}
      libhal_volume_free(volume);
    }
  libhal_free_string_array(volumes);
  return (1);
}

LibHalVolume		*pusb_volume_find(t_pusb_options *opts, LibHalContext *ctx,
					  LibHalDrive *drive)
{
    LibHalVolume	*volume = NULL;
    int			maxtries = 0;
    int			i;

    maxtries = ((opts->probe_timeout * 1000000) / 250000);
    for (i = 0; i < maxtries; ++i)
      {
	log_debug("Waiting for volumes to come up...\n");
	if (!__pusb_volume_find(opts, ctx, drive, &volume))
	  return (NULL);
	if (volume)
	  break;
	usleep(250000);
      }
    return (volume);
}

void			pusb_volume_destroy(LibHalVolume *volume)
{
  const char	*mntpoint;

  mntpoint = libhal_volume_get_mount_point(volume);
  if (mntpoint && strstr(mntpoint, "pam_usb"))
    {
      char	command[1024];

      log_debug("Attempting to umount %s\n",
		mntpoint);
      snprintf(command, sizeof(command), "pumount %s", mntpoint);
      if (!system(command))
	log_debug("Umount succeeded.\n");
      else
	log_error("Unable to umount %s\n", mntpoint);
    }
  libhal_volume_free(volume);
}
