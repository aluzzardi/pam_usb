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

static void	pusb_dump_conf(t_pusb_options *opts)
{
  printf("\nConfiguration dump:\n");
  printf("enable:\t\t\t%d\n", opts->enable);
  printf("probe_timeout:\t\t%d\n", opts->probe_timeout);
  printf("try_otp:\t\t%d\n", opts->try_otp);
  printf("enforce_otp:\t\t%d\n", opts->enforce_otp);
  printf("debug:\t\t\t%d\n", opts->debug);
  printf("hostname:\t\t%s\n", opts->hostname);
  printf("system_otp_directory:\t%s\n", opts->system_otp_directory);
  printf("device_otp_directory:\t%s\n", opts->device_otp_directory);
}

int	main(int argc, char **argv)
{
  t_pusb_options	opts;

  if (argc < 3)
    {
      printf("Usage: %s <username> <service>\n", argv[0]);
      return (1);
    }
  pusb_conf_init(&opts);
  if (!pusb_conf_parse("conf.xml", &opts, argv[1], argv[2]))
    return (0);
  pusb_dump_conf(&opts);
  if (!opts.enable)
    {
      printf("not enabled, exiting\n");
      return (0);
    }
  printf("\n");
  printf ("Access %s.\n", pusb_device_check(&opts) ? "granted" : "denied");
  return (0);
}
