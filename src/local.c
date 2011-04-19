/*
 * Copyright (c) 2003-2007 Andrea Luzzardi <scox@sig11.org>
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
#include <unistd.h>
#include <utmp.h>
#include "log.h"
#include "conf.h"

int pusb_local_login(t_pusb_options *opts, const char *user)
{
	struct utmp	utsearch;
	struct utmp	*utent;
	const char	*from;
	int			i;

	if (!opts->deny_remote)
	{
	  log_debug("deny_remote is disabled. Skipping local check.\n");
	  return (1);
	}
	log_debug("Checking whether the caller is local or not...\n");
	from = ttyname(STDIN_FILENO);
	if (!from || !(*from))
	{
		log_debug("Couldn't retrieve the tty name, aborting.\n");
		return (1);
	}
	if (!strncmp(from, "/dev/", strlen("/dev/")))
		from += strlen("/dev/");
	log_debug("Authentication request from tty %s\n", from);
	strncpy(utsearch.ut_line, from, sizeof(utsearch.ut_line) - 1);
	setutent();
	utent = getutline(&utsearch);
	endutent();
	if (!utent)
	{
		log_debug("No utmp entry found for tty \"%s\"\n",
				from);
		return (1);
	}
	for (i = 0; i < 4; ++i)
	{
		if (utent->ut_addr_v6[i] != 0)
		{
			log_error("Remote authentication request: %s\n", utent->ut_host);
			return (0);
		}
	}
	log_debug("Caller is local (good)\n");
	return (1);
}
