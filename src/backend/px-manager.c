/* px-manager.c
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

#include "config.h"

#include "px-manager.h"
#include "px-module.h"
#include "px-config-module.h"
#include "px-pacrunner-module.h"

#include <gio/gio.h>
#include <libsoup/soup.h>

enum {
  PROP_0,
  PROP_MODULE_PATH,
  LAST_PROP
};

static GParamSpec *obj_properties[LAST_PROP];

/**
 * PxManager:
 *
 * Manage libproxy modules
 */

struct _PxManager {
  GObject parent_instance;
  GList *modules;
  char *module_path;
  GCancellable *cancellable;
  SoupSession *session;
};

G_DEFINE_TYPE (PxManager, px_manager, G_TYPE_OBJECT)

G_DEFINE_QUARK (px - manager - error - quark, px_manager_error)

static void
px_manager_constructed (GObject *object)
{
  PxManager *self = PX_MANAGER (object);
  g_autoptr (GFile) module_dir = g_file_new_for_path (self->module_path);
  g_autoptr (GError) error = NULL;
  GFileEnumerator *enumerator;

  if (!module_dir) {
    g_warning ("Error accessing module path: %s. Abort!", g_file_peek_path (module_dir));
    return;
  }

  enumerator = g_file_enumerate_children (module_dir,
                                          G_FILE_ATTRIBUTE_STANDARD_NAME,
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                          self->cancellable,
                                          NULL);

  if (!enumerator) {
    g_warning ("Error enumerating module path: %s. Abort!", g_file_peek_path (module_dir));
    return;
  }

  g_print ("Loading modules from %s\n", self->module_path);

  while (TRUE) {
    GFileInfo *info;
    GFile *child;
    PxModule *module;

    if (!g_file_enumerator_iterate (enumerator, &info, &child, NULL, &error)) {
      g_warning ("Error enumerating module directory: %s", error->message);
      break;
    }

    if (!info)
      break;

    if (!g_str_has_suffix (g_file_peek_path (child), ".so"))
      continue;

    module = px_module_load (child, info, self);
    self->modules = g_list_append (self->modules, module);
  }
}

static void
px_manager_dispose (GObject *object)
{
  PxManager *self = PX_MANAGER (object);

  g_cancellable_cancel (self->cancellable);
  g_clear_object (&self->cancellable);
  g_clear_pointer (&self->module_path, g_free);

  g_clear_list (&self->modules, g_object_unref);

  G_OBJECT_CLASS (px_manager_parent_class)->dispose (object);
}

static void
px_manager_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  PxManager *self = PX_MANAGER (object);

  switch (prop_id) {
    case PROP_MODULE_PATH:
      self->module_path = g_strdup (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
px_manager_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  switch (prop_id) {
    case PROP_MODULE_PATH:
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
px_manager_class_init (PxManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = px_manager_constructed;
  object_class->dispose = px_manager_dispose;
  object_class->set_property = px_manager_set_property;
  object_class->get_property = px_manager_get_property;

  obj_properties[PROP_MODULE_PATH] = g_param_spec_string ("module-path",
                                                          "Module Path",
                                                          "The path where modules are stored.",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, obj_properties);
}

static void
px_manager_init (PxManager *self)
{
  self->session = soup_session_new ();
}

/**
 * px_manager_new:
 *
 * Create a new `PxManager`.
 *
 * Returns: the newly created `PxManager`
 */
PxManager *
px_manager_new (void)
{
  return g_object_new (PX_TYPE_MANAGER, "module-path", PX_MODULE_DIR, NULL);
}

/**
 * px_manager_pac_download:
 * @self: a px manager
 * @uri: PAC uri
 *
 * Downloads a PAC file from provided @url.
 *
 * Returns: (nullable): a newly created `GBytes` containing PAC data, or %NULL on error.
 */
GBytes *
px_manager_pac_download (PxManager  *self,
                         const char *uri)
{
  g_autoptr (SoupMessage) msg = soup_message_new (SOUP_METHOD_GET, uri);
  g_autoptr (GError) error = NULL;
  g_autoptr (GBytes) bytes = NULL;

  g_print ("%s: ENTER %s\n", __FUNCTION__, uri);
#if SOUP_CHECK_VERSION (2, 99, 4)
  bytes = soup_session_send_and_read (
    self->session,
    msg,
    NULL,     /* Pass a GCancellable here if you want to cancel a download */
    &error);
#else
  soup_session_send_message (self->session, msg);
  bytes = g_bytes_new_static (msg->response_body->data, msg->response_body->length);
#endif
  if (!bytes) {
    g_debug ("Failed to download: %s\n", error ? error->message : "");
    g_warning ("Cannot download pac file: %s", error ? error->message : "");
    return NULL;
  }

  /* g_debug ("Downloaded %zu bytes\n", g_bytes_get_size (bytes)); */
  /* g_print ("Data: %s\n", (char*)g_bytes_get_data (bytes, NULL)); */

  return g_steal_pointer (&bytes);
}

/**
 * px_manager_get_configuration:
 * @self: a px manager
 * @uri: PAC uri
 * @error: a #GError
 *
 * Get raw proxy configuration for gien @uri.
 *
 * Returns: (nullable): a newly created `GStrb` containing configuration data for @uri.
 */
char **
px_manager_get_configuration (PxManager  *self,
                              GUri       *uri,
                              GError    **error)
{
  g_auto (GStrv) config = NULL;
  GList *list = self->modules;
  const char *requested_config_module;

  requested_config_module = g_getenv ("PX_CONFIG_MODULE");

  for (; list && list->data; list = list->next) {
    PxModule *module = PX_MODULE (list->data);
    PxConfigModule *conf_module = NULL;
    PxConfigModuleInterface *iface = NULL;

    if (!PX_IS_CONFIG_MODULE (module))
      continue;

    if (requested_config_module && g_strcmp0 (requested_config_module, px_module_get_name (module)) != 0)
      continue;

    /* First config module wins at the moment */
    conf_module = PX_CONFIG_MODULE (module);
    iface = PX_CONFIG_MODULE_GET_INTERFACE (conf_module);

    if (!iface->check_available ())
      continue;

    config = iface->get_config (module, uri, error);
    return g_steal_pointer (&config);
  }

  return NULL;
}

/**
 * px_manager_get_proxies_sync:
 * @self: a px manager
 * @url: a url
 *
 * Get proxies for giben @url.
 *
 * Returns: (nullable): a newly created `GStrv` containing proxy related information.
 */
char **
px_manager_get_proxies_sync (PxManager   *self,
                             const char  *url,
                             GError     **error)
{
  GList *list;
  g_autoptr (GUri) uri = g_uri_parse (url, G_URI_FLAGS_PARSE_RELAXED, error);
  g_auto (GStrv) config = NULL;
  g_auto (GStrv) ret = NULL;
  char *pac;
  GBytes *pac_bytes;
  GTimer *timer;

  timer = g_timer_new ();
  if (!uri)
    return NULL;

  config = px_manager_get_configuration (self, uri, error);
  if (!config)
    return NULL;

  pac_bytes = px_manager_pac_download (self, config[0]);
  pac = g_strdup (g_bytes_get_data (pac_bytes, NULL));

  g_debug ("PAC loaded: %f\n", g_timer_elapsed (timer, NULL));

  for (list = self->modules; list && list->data; list = list->next) {
    PxModule *module = PX_MODULE (list->data);

    /* First pacrunner module wins at the moment */
    if (PX_IS_PACRUNNER_MODULE (module)) {
      PxPacrunnerModule *pacrunner_module = PX_PACRUNNER_MODULE (module);
      PxPacrunnerModuleInterface *iface = PX_PACRUNNER_MODULE_GET_INTERFACE (pacrunner_module);
      char *pac_ret;

      iface->set_pac (module, pac);
      g_debug ("PAC set: %f\n", g_timer_elapsed (timer, NULL));
      pac_ret = iface->run (module, uri);

      ret = g_malloc0 (sizeof (char *) * 2);
      ret[0] = g_strdup (pac_ret);
      break;
    }
  }
  g_debug ("PAC parsed: %f\n", g_timer_elapsed (timer, NULL));

  return g_steal_pointer (&ret);
}
