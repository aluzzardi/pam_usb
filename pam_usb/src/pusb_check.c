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
#include "conf.h"
#include "log.h"
#include "device.h"
#include "local.h"

static void	pusb_conf_dump(t_pusb_options *opts)
{
  fprintf(stdout, "Configuration dump:\n");
  fprintf(stdout, "enable\t\t\t: %s\n", opts->enable ? "true" : "false");
  fprintf(stdout, "debug\t\t\t: %s\n", opts->debug ? "true" : "false");
  fprintf(stdout, "quiet\t\t\t: %s\n", opts->quiet ? "true" : "false");
  fprintf(stdout, "color_log\t\t: %s\n", opts->color_log ? "true" : "false");
  fprintf(stdout, "one_time_pad\t\t: %s\n",
	  opts->one_time_pad ? "true" : "false");
  fprintf(stdout, "probe_timeout\t\t: %d\n", opts->probe_timeout);
  fprintf(stdout, "hostname\t\t: %s\n", opts->hostname);
  fprintf(stdout, "system_pad_directory\t: %s\n",
	  opts->system_pad_directory);
  fprintf(stdout, "device_pad_directory\t: %s\n",
	  opts->device_pad_directory);
}

int			main(int argc, char **argv)
{
  t_pusb_options	opts;
  int			retval;

  if (argc < 3)
    {
      printf("Usage: %s <username> <service>\n", argv[0]);
      return (1);
    }
  log_info("Authentication request for user \"%s\" (%s)\n",
	   argv[1], argv[2]);
  pusb_conf_init(&opts);
  if (!pusb_conf_parse("pusb.conf", &opts, argv[1], argv[2]))
    return (0);
  pusb_log_init(&opts);
  pusb_conf_dump(&opts);
  if (!opts.enable)
    {
      log_debug("Not enabled, exiting...\n");
      return (0);
    }
  if (!pusb_local_login(&opts, argv[1]))
    {
      log_error("Access denied.\n");
      return (0);
    }
  retval = pusb_device_check(&opts, argv[1]);
  if (retval)
    log_info("Access granted.\n");
  else
    log_error("Access denied.\n");
  return (0);
}
