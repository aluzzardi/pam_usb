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
#include <unistd.h>
#include "conf.h"
#include "log.h"
#include "device.h"
#include "local.h"

static void	pusb_check_conf_dump(t_pusb_options *opts)
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

static int	pusb_check_perform_authentication(t_pusb_options *opts,
						  const char *user,
						  const char *service)
{
  int		retval;

  if (!opts->enable)
    {
      log_debug("Not enabled, exiting...\n");
      return (0);
    }
  log_info("Authentication request for user \"%s\" (%s)\n",
	   user, service);
  if (!pusb_local_login(opts, user))
    {
      log_error("Access denied.\n");
      return (0);
    }
  retval = pusb_device_check(opts, user);
  if (retval)
    log_info("Access granted.\n");
  else
    log_error("Access denied.\n");
  return (retval);
}

static void	pusb_check_usage(const char *name)
{
  fprintf(stderr, "Usage: %s [-c <config file>] -u <username> -s <service>" \
	  " [options]\n", name);
  fprintf(stderr, "Options can be one or more of the followings:\n");
  fprintf(stderr, "\t-a Authenticate: Try to authenticate the user\n");
  fprintf(stderr, "\t-d Dump: Parse and dump the settings\n");
  fprintf(stderr, "\t-q Quiet: Silent mode\n");
}

int			main(int argc, char **argv)
{
  t_pusb_options	opts;
  char			*conf_file = PUSB_CONF_FILE;
  int			quiet = 0;
  char			*user = NULL;
  char			*service = NULL;
  int			opt;
  int			mode = 0;
  extern char		*optarg;

  while ((opt = getopt(argc, argv, "u:s:c:qad")) != EOF)
    {
      switch (opt)
	{
	case 'u':
	  user = optarg;
	  break;
	case 's':
	  service = optarg;
	  break;
	case 'c':
	  conf_file = optarg;
	  break;
	case 'q':
	  quiet = 1;
	  break;
	case 'a':
	  mode = 1;
	  break;
	case 'd':
	  mode = 2;
	  break;
	default:
	  break;
	}
    }
  if (!user || !service || !mode)
    {
      pusb_check_usage(argv[0]);
      return (1);
    }
  pusb_conf_init(&opts);
  if (!pusb_conf_parse(conf_file, &opts, user, service))
    return (1);
  opts.quiet = quiet;
  pusb_log_init(&opts);
  if (mode == 1)
    return (!pusb_check_perform_authentication(&opts, user, service));
  else if (mode == 2)
    pusb_check_conf_dump(&opts);
  return (0);
}
