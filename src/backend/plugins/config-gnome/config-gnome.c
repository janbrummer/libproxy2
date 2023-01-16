/* config-gnome.c
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
#include <glib.h>

#include "config-gnome.h"
#include "px-plugin-config.h"

struct _PxConfigGnome {
  GObject parent_instance;
  GSettings *proxy_settings;
};

enum {
  GNOME_PROXY_MODE_AUTO = 2
};

static void px_config_iface_init (PxConfigInterface *iface);
void peas_register_types (PeasObjectModule *module);

G_DEFINE_FINAL_TYPE_WITH_CODE (PxConfigGnome,
                               px_config_gnome,
                               G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (PX_TYPE_CONFIG, px_config_iface_init))

static void
on_proxy_settings_changed (GSettings *self,
                           gchar     *key,
                           gpointer   user_data)
{
  int mode;
  char *server;

  mode = g_settings_get_enum (self, "mode");

  /* Automatic */
  if (mode == 2) {
    server = g_settings_get_string (self, "autoconfig-url");
  } else {
    server = g_strdup ("");
  }

  g_print ("Server: %s\n", server);
}

static void
px_config_gnome_init (PxConfigGnome *self)
{
}

static void
px_config_gnome_class_init (PxConfigGnomeClass *klass)
{
}

static gboolean
px_config_gnome_is_available (PxConfig *config)
{
  return (g_getenv ("GNOME_DESKTOP_SESSION_ID") ||
          (g_strcmp0 (g_getenv ("DESKTOP_SESSION"), "gnome") == 0) ||
          (g_strcmp0 (g_getenv ("DESKTOP_SESSION"), "mate") == 0));
}

static GStrv
px_config_gnome_get_config (PxConfig  *config,
                            GUri      *uri,
                            GError   **error)
{
  PxConfigGnome *self = PX_CONFIG_GNOME (config);
  GStrv ret = NULL;
  int mode;
  g_autofree char *server = NULL;

  if (!self->proxy_settings) {
    self->proxy_settings = g_settings_new ("org.gnome.system.proxy");
    g_signal_connect (self->proxy_settings, "changed", G_CALLBACK (on_proxy_settings_changed), NULL);
    on_proxy_settings_changed (self->proxy_settings, NULL, self);
  }

  mode = g_settings_get_enum (self->proxy_settings, "mode");
  if (mode == GNOME_PROXY_MODE_AUTO) {
    char *autoconfig_url = g_settings_get_string (self->proxy_settings, "autoconfig-url");

    if (strlen (autoconfig_url) != 0) {
      server = g_strdup_printf ("pac+%s", autoconfig_url);
    } else {
      server = g_strdup ("wpad://");
    }
  } else {
    server = g_strdup ("direct://");
  }

  ret = g_malloc0 (sizeof (char *) * 2);
  ret[0] = g_strdup (server);

  return ret;
}

static void
px_config_iface_init (PxConfigInterface *iface)
{
  iface->is_available = px_config_gnome_is_available;
  iface->get_config = px_config_gnome_get_config;
}

void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PX_TYPE_CONFIG,
                                              PX_CONFIG_TYPE_GNOME);
}
