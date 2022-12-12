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

#include <glib-object.h>

#include "config-gnome.h"
#include "px-module.h"
#include "px-config-module.h"

struct _PxGnomeModule {
  GObject parent_instance;
  GSettings *proxy_settings;
};

enum {
  GNOME_PROXY_MODE_AUTO = 2
};

static void module_interface_init (gpointer g_iface,
                                   gpointer data);

G_DEFINE_TYPE_WITH_CODE (PxGnomeModule, px_gnome_module,
                         PX_TYPE_MODULE,
                         G_IMPLEMENT_INTERFACE (PX_TYPE_CONFIG_MODULE, module_interface_init);
                         );

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
    server = g_strdup ("http://moep");
  }

  g_print ("Server: %s\n", server);
}

static void
px_gnome_module_init (PxGnomeModule *self)
{
}

static void
px_gnome_module_class_init (PxGnomeModuleClass *klass)
{
}

static gboolean
gnome_check_available (void)
{
  return (g_getenv ("GNOME_DESKTOP_SESSION_ID") ||
          (g_strcmp0 (g_getenv ("DESKTOP_SESSION"), "gnome") == 0) ||
          (g_strcmp0 (g_getenv ("DESKTOP_SESSION"), "mate") == 0));
}

static GStrv
gnome_get_config (PxModule  *module,
                  GUri      *uri,
                  GError   **error)
{
  PxGnomeModule *self = PX_GNOME_MODULE (module);
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
module_interface_init (gpointer g_iface,
                       gpointer data)
{
  PxConfigModuleInterface *iface = g_iface;

  iface->name = "GNOME";
  iface->version = 1;
  iface->check_available = gnome_check_available;
  iface->get_config = gnome_get_config;
}

PxModule *
px_module_create (void)
{
  return g_object_new (PX_TYPE_GNOME_MODULE, NULL);
}
