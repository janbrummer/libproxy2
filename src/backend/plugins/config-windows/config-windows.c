/* config-windows.c
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

#include <windows.h>
#include <winreg.h>
#include <libpeas/peas.h>

#include "config-windows.h"

#include "px-plugin-config.h"
#include "px-manager.h"

#define W32REG_OFFSET_PAC (1 << 2)
#define W32REG_OFFSET_WPAD (1 << 3)
#define W32REG_BASEKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"
#define W32REG_BUFFLEN 1024

static void px_config_iface_init (PxConfigInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (PxConfigWindows,
                                px_config_windows,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PX_TYPE_CONFIG, px_config_iface_init))

static void
px_config_windows_init (PxConfigWindows *self)
{
  g_debug ("%s", G_STRFUNC);
}

static void
px_config_windows_class_init (PxConfigWindowsClass *klass)
{
}

static void
px_config_windows_class_finalize (PxConfigWindowsClass *klass)
{
}

static gboolean
px_config_windows_is_available (PxConfig *self)
{
  g_print ("%s: ENTER\n", __FUNCTION__);
  return TRUE;
}

static gboolean
get_registry (const char  *key,
              const char  *name,
              char       **sval,
              guint32     *slen,
              guint32     *ival)
{
  HKEY hkey;
  LONG result;
  DWORD type;
  DWORD buflen = W32REG_BUFFLEN;
  BYTE buffer[W32REG_BUFFLEN];

  if (sval && ival)
    return FALSE;

  if (RegOpenKeyExA (HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
    return FALSE;

  result = RegQueryValueExA (hkey, name, NULL, &type, buffer, &buflen);
  RegCloseKey (hkey);

  if (result != ERROR_SUCCESS)
    return FALSE;

  switch (type) {
    case REG_BINARY:
    case REG_EXPAND_SZ:
    case REG_SZ:
      if (!sval)
        return FALSE;
      if (slen)
        *slen = buflen;

      *sval = g_malloc0 (buflen);
      return memcpy (*sval, buffer, buflen) != NULL;
    case REG_DWORD:
      if (ival)
        return memcpy (ival, buffer, buflen < sizeof (guint32) ? buflen : sizeof (guint32)) != NULL;
    default:
      break;
  }

  return FALSE;
}

static gboolean
is_enabled (char type)
{
  g_autofree char *data = NULL;
  guint32 dlen = 0;
  gboolean result = FALSE;

  if (!get_registry (W32REG_BASEKEY "\\Connections", "DefaultConnectionSettings", &data, &dlen, NULL))
    return FALSE;

  if (dlen >= 9)
    result = (data[8] & type) == type;

  return result;
}

static char **
px_config_windows_get_config (PxConfig  *self,
                              GUri      *uri,
                              GError   **error)
{
  g_autoptr(GStrvBuilder) builder = g_strv_builder_new ();)
  char *tmp = NULL;
  guint32 enabled = 0;

  if (get_registry (, "ProxyOverride", &tmp, NULL, NULL)) {
    g_print ("Override: %s\n", tmp);
  }

  /* WPAD */
  if (is_enabled (W32REG_OFFSET_WPAD)) {
    g_strv_builder_add (builder, "wpad://");
    return g_strv_builder_end (builder);
  }

  /* PAC */
  if (is_enabled (W32REG_OFFSET_PAC) && get_registry (W32REG_BASEKEY, "AutoConfigURL", &tmp, NULL, NULL)) {
    g_autofree char *pac_uri = g_strconcat ("pac+", tmp, NULL);
    GUri *ac_uri = g_uri_parsed (tmp, G_URI_FLAGS_RELAXED, NULL);

    if (ac_uri) {
      g_strv_builder_add (builder, pac_uri);
      return g_strv_builder_end (builder);
    }
  }

  /* Manual proxy */
  if (get_registry (W32REG_BASEKEY, "ProxyEnable", NULL, NULL, &enabled) && enabled && get_registry (W32REG_BASEKEY, "ProxyServer", &tmp, NULL, NULL)) {
    g_print ("tmp: %s\n", tmp);

    /* TODO */
    return g_strv_builder_end (builder);
  }

  /* Direct */
  g_strv_builder_add (builder, "direct://");
  return g_strv_builder_end (builder);
}

static void
px_config_iface_init (PxConfigInterface *iface)
{
  iface->is_available = px_config_windows_is_available;
  iface->get_config = px_config_windows_get_config;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  px_config_windows_register_type (G_TYPE_MODULE (module));
  peas_object_module_register_extension_type (module,
                                              PX_TYPE_CONFIG,
                                              PX_CONFIG_TYPE_WINDOWS);
}
