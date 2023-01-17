/* config-osx.c
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

#include <libpeas/peas.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include "config-osx.h"

#include "px-plugin-config.h"
#include "px-manager.h"

static void px_config_iface_init (PxConfigInterface *iface);
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_DEFINE_FINAL_TYPE_WITH_CODE (PxConfigOsX,
                               px_config_osx,
                               G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (PX_TYPE_CONFIG, px_config_iface_init))

static void
px_config_osx_init (PxConfigOsX *self)
{
}

static void
px_config_osx_class_init (PxConfigOsXClass *klass)
{
}

static gboolean
px_config_osx_is_available (PxConfig *self)
{
  return TRUE;
}

static CFNumberRef
getobj (CFDictionaryRef  settings,
        char            *key)
{
  CFStringRef k;
  CFNumberRef retval;

  if (!settings)
    return NULL;

  k = CFStringCreateWithCString (NULL, key.c_str(), kCFStringEncodingMacRoman);
  if (!k)
    return NULL;

  retval = (CFNumberRef) CFDictionaryGetValue(settings, k);

  CFRelease (k);
  return retval;
}

static gboolean
getint (CFDictionaryRef  settings,
        char            *key,
        int64_t         *answer)
{
  CFNumberRef n = getobj<CFNumberRef>(settings, key);

  if (!n)
    return FALSE;

  if (!CFNumberGetValue (n, kCFNumberSInt64Type, &answer))
    return FALSE;

  return TRUE;
}

static gboolean
getbool (CFDictionaryRef  settings,
         char            *key)
{
  int64_t i;

  if (!getint(settings, key, i))
    return FALSE;

  return i != 0;
}

static gboolean
px_config_osx_get_config (PxConfig      *self,
                          GUri          *uri,
                          GStrvBuilder  *builder,
                          GError       **error)
{
  const char *proxy = NULL;
  CFDictionaryRef proxies = SCDynamicStoreCopyProxies (NULL);

  if (!proxies) {
    g_warning ("Unable to fetch proxy configuration");
    return FALSE;
  }

  if (getbool(proxies, "ProxyAutoDiscoveryEnable")) {
    CFRelease (proxies);
    g_strv_builder_add (builder, "wpad://");
    return TRUE;
  }

  if (getbool (proxies, "ProxyAutoConfigEnable")) {
    CFRelease (proxies);
    g_strv_builder_add (builder, "pac+");
    return TRUE;
  }

  if (proxy)
    g_strv_builder_add (builder, proxy);

  return TRUE;
}

static void
px_config_iface_init (PxConfigInterface *iface)
{
  iface->is_available = px_config_osx_is_available;
  iface->get_config = px_config_osx_get_config;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PX_TYPE_CONFIG,
                                              PX_CONFIG_TYPE_OSX);
}
