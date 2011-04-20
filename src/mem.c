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

#include <assert.h>
#include "mem.h"

void *xmalloc(size_t size)
{
	void *data = malloc(size);
	assert(data != NULL && "malloc() failed");
	return (data);
}

void *xrealloc(void *ptr, size_t size)
{
	void *data = realloc(ptr, size);
	assert(data != NULL && "realloc() failed");
	return (data);
}

void xfree(void *ptr)
{
	free(ptr);
}

char *xstrdup(const char *s)
{
	char *data = strdup(s);
	assert(data != NULL && "strdup() failed");
	return (data);
}
