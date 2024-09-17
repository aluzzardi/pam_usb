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

#ifndef VOLUME_H_
# define VOLUME_H_
# include <udisks/udisks.h>
# include "conf.h"

typedef struct pusb_volume
{
	UDisksFilesystem *filesystem;
	int unmount;
	char *device;
	char *mount_point;
} t_pusb_volume;

t_pusb_volume *pusb_volume_get(t_pusb_options *opts, UDisksClient *udisks);
void pusb_volume_destroy(t_pusb_volume *volume);

#endif /* !VOLUME_H_ */
