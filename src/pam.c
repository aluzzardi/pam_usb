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

#define PAM_SM_AUTH
#include <security/pam_modules.h>
#include <security/_pam_macros.h>

#include "version.h"
#include "conf.h"
#include "log.h"
#include "local.h"
#include "device.h"

PAM_EXTERN
int			pam_sm_authenticate(pam_handle_t *pamh, int flags,
					    int argc, const char **argv)
{
  t_pusb_options	opts;
  const char		*service;
  const char		*user;
  const char		*tty;
  char			*conf_file = PUSB_CONF_FILE;
  int			retval;

  retval = pam_get_item(pamh, PAM_SERVICE, (const void **)&service);
  if (retval != PAM_SUCCESS)
    {
      log_error("Unable to retrieve the PAM service name.\n");
      return (PAM_AUTH_ERR);
    }

  if (pam_get_user(pamh, &user, NULL) != PAM_SUCCESS || !user || !*user)
    {
      log_error("Unable to retrieve the PAM user name.\n");
      return (PAM_AUTH_ERR);
    }

  if (argc > 1)
    if (!strcmp(argv[0], "-c"))
      conf_file = (char *)argv[1];
  pusb_conf_init(&opts);
  if (!pusb_conf_parse(conf_file, &opts, user, service))
    return (PAM_AUTH_ERR);

  pusb_log_init(&opts);
  if (!opts.enable)
    {
      log_debug("Not enabled, exiting...\n");
      return (PAM_IGNORE);
    }

  log_info("pam_usb v%s\n", PUSB_VERSION);
  log_info("Authentication request for user \"%s\" (%s)\n",
	   user, service);

  if (pam_get_item(pamh, PAM_TTY, (const void **)&tty) == PAM_SUCCESS)
    {
      if (!strcmp(tty, "ssh"))
	{
	  log_debug("SSH Authentication, aborting.\n");
	  return (0);
	}
    }
  if (!pusb_local_login(&opts, user))
    {
      log_error("Access denied.\n");
      return (PAM_AUTH_ERR);
    }
  if (pusb_device_check(&opts, user))
    {
      log_info("Access granted.\n");
      return (PAM_SUCCESS);
    }
  log_error("Access denied.\n");
  return (PAM_AUTH_ERR);
}

PAM_EXTERN
int	pam_sm_setcred(pam_handle_t *pamh,int flags,int argc,
		       const char **argv)
{
  return (PAM_SUCCESS);
}

#ifdef PAM_STATIC

struct pam_module _pam_usb_modstruct = {
  "pam_usb",
  pam_sm_authenticate,
  pam_sm_setcred,
  NULL,
  NULL,
  NULL,
  NULL
};

#endif
