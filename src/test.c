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
  if (!pusb_conf_parse("conf.xml", &opts, argv[1], argv[2]))
    return (0);
  if (!opts.enable)
    {
      log_debug("Not enabled, exiting...\n");
      return (0);
    }
  retval = pusb_device_check(&opts, argv[1]);
  if (retval)
    log_info("Access granted.\n");
  else
    log_error("Access denied.\n");
  return (0);
}
