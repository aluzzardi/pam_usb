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

#include <libxml/xpath.h>
#include <string.h>
#include "xpath.h"
#include "log.h"

static xmlXPathObject	*pusb_xpath_match(xmlDocPtr doc, const char *path)
{
  xmlXPathContext	*context = NULL;
  xmlXPathObject	*result = NULL;

  context = xmlXPathNewContext(doc);
  if (context == NULL)
    {
      log_error("Unable to create XML context\n");
      return (NULL);
    }
  result = xmlXPathEvalExpression((xmlChar *)path, context);
  xmlXPathFreeContext(context);
  if (result == NULL)
    {
      log_error("Error in xmlXPathEvalExpression\n");
      return (NULL);
    }
  if (xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
      xmlXPathFreeObject(result);
      return (NULL);
    }
  return (result);
}

int			pusb_xpath_get_string(xmlDocPtr doc, const char *path,
					      char *value, size_t size)
{
  xmlXPathObject	*result = NULL;
  xmlNode		*node = NULL;
  xmlChar		*result_string = NULL;

  if (!(result = pusb_xpath_match(doc, path)))
    return (0);

  if (result->nodesetval->nodeNr > 1)
    {
      xmlXPathFreeObject(result);
      log_debug("Syntax error: %s: more than one record found\n", path);
      return (0);
    }

  node = result->nodesetval->nodeTab[0]->xmlChildrenNode;
  result_string = xmlNodeListGetString(doc, node, 1);
  if (strlen((const char *)result_string) + 1 > size)
    {
      xmlFree(result_string);
      xmlXPathFreeObject(result);
      log_debug("Result for %s (%s) is too long (max: %d)\n",
		path, (const char *)result_string, size);
      return (0);
    }
  memset(value, '\0', size);
  strncpy(value, (const char *)result_string, size);
  xmlFree(result_string);
  xmlXPathFreeObject(result);
  return (1);
}

int			pusb_xpath_get_string_from(xmlDocPtr doc,
						   const char *base,
						   const char *path,
						   char *value, size_t size)
{
  char			*xpath = NULL;
  size_t		xpath_size;
  int			retval;

  xpath_size = strlen(base) + strlen(path) + 1;
  if (!(xpath = malloc(xpath_size)))
    {
      log_error("malloc error !\n");
      return (0);
    }
  memset(xpath, 0x00, xpath_size);
  snprintf(xpath, xpath_size, "%s%s", base, path);
  retval = pusb_xpath_get_string(doc, xpath, value, size);
  if (retval)
    log_debug("%s%s -> %s\n", base, path, value);
  free(xpath);
  return (retval);
}

int	pusb_xpath_get_bool(xmlDocPtr doc, const char *path, int *value)
{
  char	ret[6]; /* strlen("false") + 1 */

  if (!pusb_xpath_get_string(doc, path, ret, sizeof(ret)))
    return (0);

  if (!strcmp(ret, "true"))
    {
      *value = 1;
      return (1);
    }

  if (!strcmp(ret, "false"))
    {
      *value = 0;
      return (1);
    }

  log_debug("Expecting a boolean, got %s\n", ret);
  return (0);
}

int			pusb_xpath_get_bool_from(xmlDocPtr doc,
						 const char *base,
						 const char *path,
						 int *value)
{
  char		*xpath = NULL;
  size_t	xpath_size;
  int		retval;

  xpath_size = strlen(base) + strlen(path) + 1;
  if (!(xpath = malloc(xpath_size)))
    {
      log_error("malloc error!\n");
      return (0);
    }
  memset(xpath, 0x00, xpath_size);
  snprintf(xpath, xpath_size, "%s%s", base, path);
  retval = pusb_xpath_get_bool(doc, xpath, value);
  free(xpath);
  if (retval)
    log_debug("%s%s -> %s\n", base, path, *value ? "true" : "false");
  return (retval);
}

int		pusb_xpath_get_int(xmlDocPtr doc, const char *path, int *value)
{
  char		ret[64]; /* strlen("false") + 1 */

  if (!pusb_xpath_get_string(doc, path, ret, sizeof(ret)))
    return (0);
  *value = atoi(ret);
  return (1);
}

int			pusb_xpath_get_int_from(xmlDocPtr doc,
						const char *base,
						const char *path,
						int *value)
{
  char		*xpath = NULL;
  size_t	xpath_size;
  int		retval;

  xpath_size = strlen(base) + strlen(path) + 1;
  if (!(xpath = malloc(xpath_size)))
    {
      log_error("malloc error!\n");
      return (0);
    }
  memset(xpath, 0x00, xpath_size);
  snprintf(xpath, xpath_size, "%s%s", base, path);
  retval = pusb_xpath_get_int(doc, xpath, value);
  free(xpath);
  if (retval)
    log_debug("%s%s -> %d\n", base, path, *value);
  return (retval);
}
