/* px-module.c
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

#include "px-module.h"
#include "px-config-module.h"
#include "px-pacrunner-module.h"

G_DEFINE_QUARK (px - module - error - quark, px_module_error)

struct _PxModule {
  GObject parent_instance;
};

G_DEFINE_TYPE (PxModule, px_module, G_TYPE_OBJECT)

static void
px_module_class_init (PxModuleClass *klass)
{
}

static void
px_module_init (PxModule *self)
{
}

/**
 * px_module_load:
 * @target: a target #GFile to load
 * @info: a #GFileInfo for detailed information
 * @user_data: unused
 *
 * Load a px module from disk and wrap it into a #PxModule.
 *
 * Returns: (nullable): a newly created `PxModule`.
 */
PxModule *
px_module_load (GFile     *target,
                GFileInfo *info,
                gpointer   user_data)
{
  GModule *module;
  gpointer func;
  PxModuleCreate create;
  PxModule *px_module;

  g_assert (target);
  g_assert (info);

  g_debug ("%s: Loading %s\n", __FUNCTION__, g_file_peek_path (target));
  module = g_module_open (g_file_peek_path (target), G_MODULE_BIND_LAZY);
  if (!module) {
    g_warning ("%s: failed for %s", __FUNCTION__, g_file_peek_path (target));
    return NULL;
  }

  if (!g_module_symbol (module, "px_module_create", &func)) {
    g_warning ("Could not load module!");
    g_module_close (module);
  }

  g_module_make_resident (module);

  create = func;
  px_module = create ();

  if (!px_module) {
    g_warning ("%s: failed for %s", __FUNCTION__, g_file_peek_path (target));
    return NULL;
  }

  g_print ("%s: Loaded %s\n", __FUNCTION__, g_file_peek_path (target));
  return px_module;
}

const char *
px_module_get_name (PxModule *self)
{
  if (PX_IS_CONFIG_MODULE (self)) {
    PxConfigModuleInterface *ifc = PX_CONFIG_MODULE_GET_INTERFACE (self);

    return ifc->name;
  }

  return NULL;
}

static void
px_config_module_interface_init (PxConfigModuleInterface *iface)
{
}

GType
px_config_module_get_type (void)
{
  static GType type = 0;
  static const GTypeInfo info = {
    sizeof (PxConfigModuleInterface),
    NULL,
    NULL,
    (GClassInitFunc)px_config_module_interface_init
  };

  if (type) return type;

  type = g_type_register_static (
    G_TYPE_INTERFACE,
    "PxConfigModule",
    &info,
    0);

  return type;
}


static void
px_pacrunner_module_interface_init (PxPacrunnerModuleInterface *iface)
{
}

GType
px_pacrunner_module_get_type (void)
{
  static GType type = 0;
  static const GTypeInfo info = {
    sizeof (PxPacrunnerModuleInterface),
    NULL,
    NULL,
    (GClassInitFunc)px_pacrunner_module_interface_init
  };

  if (type) return type;

  type = g_type_register_static (
    G_TYPE_INTERFACE,
    "PxPacrunnerModule",
    &info,
    0);

  return type;
}
