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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <libhal-storage.h>
#include "conf.h"
#include "log.h"
#include "volume.h"
#include "otp.h"

static FILE	*pusb_pad_open_device(t_pusb_options *opts,
				      LibHalVolume *volume, const char *mode)
{
  FILE		*f;
  char		*path;
  size_t	path_size;
  const char	*mnt_point;

  mnt_point = (char *)libhal_volume_get_mount_point(volume);
  if (!mnt_point)
    return (NULL);
  path_size = strlen(mnt_point) + 1 + strlen(opts->device_pad_directory) + \
    1 + strlen(opts->hostname) + strlen(".pad") + 1;
  if (!(path = malloc(path_size)))
    {
      log_error("malloc error!\n");
      return (NULL);
    }
  memset(path, 0x00, path_size);
  snprintf(path, path_size, "%s/%s/%s.pad", mnt_point,
	   opts->device_pad_directory, opts->hostname);
  f = fopen(path, mode);
  free(path);
  if (!f)
    {
      log_debug("Cannot open device file: %s\n", strerror(errno));
      return (NULL);
    }
  return (f);
}

static FILE	*pusb_pad_open_system(t_pusb_options *opts, const char *mode)
{
  FILE		*f;
  char		*path;
  size_t	path_size;

  path_size = strlen(opts->system_pad_directory) + 1 +
    strlen(opts->device.serial) + strlen(".pad") + 1;
  if (!(path = malloc(path_size)))
    {
      log_error("malloc error\n");
      return (NULL);
    }
  memset(path, 0x00, path_size);
  snprintf(path, path_size, "%s/%s.pad", opts->system_pad_directory,
	   opts->device.serial);
  f = fopen(path, mode);
  free(path);
  if (!f)
    {
      log_debug("Cannot open system file: %s\n", strerror(errno));
      return (NULL);
    }
  return (f);
}

static void	pusb_pad_update(t_pusb_options *opts,
				LibHalVolume *volume)
{
  FILE		*f_device = NULL;
  FILE		*f_system = NULL;
  int		magic[1024];
  int		i;

  if (!(f_device = pusb_pad_open_device(opts, volume, "w+")))
    {
      log_error("Unable to update pads.\n");
      return ;
    }
  if (!(f_system = pusb_pad_open_system(opts, "w+")))
    {
      log_error("Unable to update pads.\n");
      fclose(f_device);
      return ;
    }
  log_debug("Generating %d bytes unique pad...\n", sizeof(magic));
  srand(getpid() * time(NULL));
  for (i = 0; i < (sizeof(magic) / sizeof(int)); ++i)
    magic[i] = rand();
  log_debug("Writing pad to the device...\n");
  fwrite(magic, sizeof(int), sizeof(magic) / sizeof(int), f_system);
  log_debug("Writing pad to the system...\n");
  fwrite(magic, sizeof(int), sizeof(magic) / sizeof(int), f_device);
  log_debug("Synchronizing filesystems...\n");
  fclose(f_system);
  fclose(f_device);
  sync();
  log_debug("One time pads updated.\n");
}

static int	pusb_pad_compare(t_pusb_options *opts, LibHalVolume *volume)
{
  FILE		*f_device = NULL;
  FILE		*f_system = NULL;
  int		magic_device[1024];
  int		magic_system[1024];
  int		retval;

  if (!(f_system = pusb_pad_open_system(opts, "r")))
    return (1);
  if (!(f_device = pusb_pad_open_device(opts, volume, "r")))
    {
      fclose(f_system);
      return (0);
    }
  log_debug("Loading device pad...\n");
  fread(magic_device, sizeof(int), sizeof(magic_device) / sizeof(int),
	f_device);
  log_debug("Loading system pad...\n");
  fread(magic_system, sizeof(int), sizeof(magic_system) / sizeof(int),
	f_system);
  retval = memcmp(magic_system, magic_device, sizeof(magic_system));
  fclose(f_system);
  fclose(f_device);
  return (retval == 0);
}

int		pusb_pad_check(t_pusb_options *opts, LibHalContext *ctx)
{
  LibHalVolume	*volume = NULL;
  int		retval;

  volume = pusb_volume_get(opts, ctx);
  if (!volume)
    return (0);
  retval = pusb_pad_compare(opts, volume);
  if (retval)
    {
      log_info("Verification match, updating one time pads...\n");
      pusb_pad_update(opts, volume);
    }
  else
    log_error("Pad checking failed !\n");
  pusb_volume_destroy(volume);
  return (retval);
}
