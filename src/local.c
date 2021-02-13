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
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include "log.h"
#include "conf.h"
#include "process.h"

int pusb_local_login(t_pusb_options *opts, const char *user)
{
	if (!opts->deny_remote)
	{
	  log_debug("deny_remote is disabled. Skipping local check.\n");
	  return (1);
	}

	log_debug("Checking whether the caller is local or not...\n");
	
	pid_t pid = getpid();
	while (pid != 0) {
		char name[BUFSIZ];
		get_process_name(pid, name);
		log_debug("    Checking pid %6d (%s)...\n", pid, name);
		get_process_parent_id(pid, & pid);

		if (strstr(name, "sshd") != NULL || strstr(name, "telnetd") != NULL) {
			log_error("One of the parent processes found to be a remote access daemon, denying.\n");
			return (0);
		}
	}

	log_debug("No remote daemons found in parent process list, seems to be local request - allowing.\n");
	return (1);
}
