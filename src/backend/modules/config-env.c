/* config-env.c
 *
 * Copyright 2022-2023 Jan-Michael Brummer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <glib-object.h>

#include "config-env.h"
#include "px-module.h"
#include "px-config-module.h"

struct _PxEnvModule {
  GObject parent_instance;
};

static void module_interface_init (gpointer g_iface,
                                   gpointer data);

G_DEFINE_TYPE_WITH_CODE (PxEnvModule, px_env_module,
                         PX_TYPE_MODULE,
                         G_IMPLEMENT_INTERFACE (PX_TYPE_CONFIG_MODULE, module_interface_init);
                         );

static void
px_env_module_init (PxEnvModule *self)
{
}

static void
px_env_module_class_init (PxEnvModuleClass *klass)
{
}

static gboolean
env_check_available (void)
{
  return TRUE;
}

static GStrv
env_get_config (PxModule  *px_module,
                GUri      *uri,
                GError   **error)
{
  g_auto (GStrv) ret = NULL;
  const char *proxy = NULL;
  const char *scheme = g_uri_get_scheme (uri);
  const char *ignore = NULL;

  ignore = g_getenv ("no_proxy");
  if (!ignore)
    ignore = g_getenv ("NO_PROXY");

  if (ignore && strstr(ignore, g_uri_get_host (uri)))
    return g_steal_pointer (&ret);

  if (g_strcmp0 (scheme, "ftp") == 0) {
    proxy = g_getenv ("ftp_proxy");
    if (!proxy)
      proxy = g_getenv ("FTP_PROXY");
  } else if (g_strcmp0 (scheme, "https") == 0) {
    proxy = g_getenv ("https_proxy");
    if (!proxy)
      proxy = g_getenv ("HTTPS_PROXY");
  }

  if (!proxy) {
    proxy = g_getenv ("http_proxy");
    if (!proxy)
      proxy = g_getenv ("HTTP_PROXY");
  }

  if (!proxy && error) {
    g_set_error (error, PX_MODULE_ERROR, PX_MODULE_ERROR_CONFIGURATION, "Unable to read environment configuration");
    return NULL;
  }

  ret = g_malloc0 (sizeof (char *) * 2);
  ret[0] = g_strdup (proxy);

  return g_steal_pointer (&ret);
}

static void
module_interface_init (gpointer g_iface,
                       gpointer data)
{
  PxConfigModuleInterface *iface = g_iface;

  iface->name = g_strdup ("Environment");
  iface->version = 1;
  iface->check_available = env_check_available;
  iface->get_config = env_get_config;
}

PxModule *
px_module_create (void)
{
  return g_object_new (PX_TYPE_ENV_MODULE, NULL);
}
