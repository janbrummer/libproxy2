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
#include "px-plugin-config.h"
#include "px-plugin-pacrunner.h"

#include <libsoup/soup.h>
#include <libpeas/peas.h>

enum {
  PROP_0,
  PROP_PLUGINS_DIR,
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
  PeasEngine *engine;
  PeasExtensionSet *config_set;
  PeasExtensionSet *pacrunner_set;
  char *plugins_dir;
  GCancellable *cancellable;
  SoupSession *session;
};

G_DEFINE_TYPE (PxManager, px_manager, G_TYPE_OBJECT)

G_DEFINE_QUARK (px - manager - error - quark, px_manager_error)

static void
px_manager_constructed (GObject *object)
{
  PxManager *self = PX_MANAGER (object);
  const GList *list;

  self->session = soup_session_new ();

  self->engine = peas_engine_get_default ();

  peas_engine_add_search_path (self->engine, self->plugins_dir, NULL);

  self->config_set = peas_extension_set_new (self->engine, PX_TYPE_CONFIG, NULL);
  self->pacrunner_set = peas_extension_set_new (self->engine, PX_TYPE_PACRUNNER, NULL);
  list = peas_engine_get_plugin_list (self->engine);
  for (; list && list->data; list = list->next) {
    PeasPluginInfo *info = PEAS_PLUGIN_INFO (list->data);
    PeasExtension *extension = peas_extension_set_get_extension (self->config_set, info);
    gboolean available = TRUE;

    if (!peas_plugin_info_is_loaded (info))
      peas_engine_load_plugin (self->engine, info);

    extension = peas_extension_set_get_extension (self->config_set, info);
    if (extension) {
      PxConfigInterface *ifc = PX_CONFIG_GET_IFACE (extension);

      available = ifc->is_available (PX_CONFIG (extension));
    }

    if (!available)
      peas_engine_unload_plugin (self->engine, info);
  }
}

static void
px_manager_dispose (GObject *object)
{
  PxManager *self = PX_MANAGER (object);

  g_cancellable_cancel (self->cancellable);
  g_clear_object (&self->cancellable);
  g_clear_pointer (&self->plugins_dir, g_free);
  g_clear_object (&self->engine);

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
    case PROP_PLUGINS_DIR:
      self->plugins_dir = g_strdup (g_value_get_string (value));
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
    case PROP_PLUGINS_DIR:
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

  obj_properties[PROP_PLUGINS_DIR] = g_param_spec_string ("plugins-dir",
                                                          "Plugins Dir",
                                                          "The directory where plugins are stored.",
                                                          NULL,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, obj_properties);
}

static void
px_manager_init (PxManager *self)
{
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
  return g_object_new (PX_TYPE_MANAGER, "plugins-dir", PX_PLUGINS_DIR, NULL);
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

  bytes = soup_session_send_and_read (
    self->session,
    msg,
    NULL,     /* Pass a GCancellable here if you want to cancel a download */
    &error);
  if (!bytes) {
    g_debug ("Failed to download: %s\n", error ? error->message : "");
    g_warning ("Cannot download pac file: %s", error ? error->message : "");
    return NULL;
  }

  return g_steal_pointer (&bytes);
}

struct ConfigData {
  GUri *uri;
  char **ret;
  GError **error;
};

static void
get_config (PeasExtensionSet *set,
            PeasPluginInfo   *info,
            PeasExtension    *extension,
            gpointer          data)
{
  PxConfigInterface *ifc = PX_CONFIG_GET_IFACE (extension);
  struct ConfigData *config_data = data;

  if (!config_data->ret)
    config_data->ret = ifc->get_config (PX_CONFIG (extension), config_data->uri, config_data->error);
}

/**
 * px_manager_get_configuration:
 * @self: a px manager
 * @uri: PAC uri
 * @error: a #GError
 *
 * Get raw proxy configuration for gien @uri.
 *
 * Returns: (nullable): a newly created `GStrv` containing configuration data for @uri.
 */
char **
px_manager_get_configuration (PxManager  *self,
                              GUri       *uri,
                              GError    **error)
{
  struct ConfigData a = {
    .uri = uri,
    .error = error,
  };

  peas_extension_set_foreach (self->config_set, get_config, &a);
  return a.ret;
}

struct PacData {
  char *pac;
  GUri *uri;
  char *ret;
};

static void
parse_pac (PeasExtensionSet *set,
           PeasPluginInfo   *info,
           PeasExtension    *extension,
           gpointer          data)
{
  PxPacRunnerInterface *ifc = PX_PAC_RUNNER_GET_IFACE (extension);
  struct PacData *pac_data = data;

  g_print ("%s: ENTER\n", __FUNCTION__);

  if (pac_data->ret)
    return;

  ifc->set_pac (PX_PAC_RUNNER (extension), pac_data->pac);
  pac_data->ret = ifc->run (PX_PAC_RUNNER (extension), pac_data->uri);
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
  /* GList *list; */
  g_autoptr (GUri) uri = g_uri_parse (url, G_URI_FLAGS_PARSE_RELAXED, error);
  g_auto (GStrv) config = NULL;
  g_auto (GStrv) ret = NULL;
  char *pac;
  GBytes *pac_bytes;
  GTimer *timer;
  struct PacData pac_data;

  timer = g_timer_new ();
  if (!uri)
    return NULL;

  config = px_manager_get_configuration (self, uri, error);
  if (!config)
    return NULL;

  /* ret = g_steal_pointer (&config); */

  pac_bytes = px_manager_pac_download (self, config[0]);
  pac = g_strdup (g_bytes_get_data (pac_bytes, NULL));

  g_debug ("PAC loaded: %f\n", g_timer_elapsed (timer, NULL));

  pac_data.pac = pac;
  pac_data.uri = uri;
  pac_data.ret = NULL;
  peas_extension_set_foreach (self->pacrunner_set, parse_pac, &pac_data);
  /* for (list = self->modules; list && list->data; list = list->next) { */
  /*   PxModule *module = PX_MODULE (list->data); */

  /* First pacrunner module wins at the moment */
  /*   if (PX_IS_PACRUNNER_MODULE (module)) { */
  /*     PxPacrunnerModule *pacrunner_module = PX_PACRUNNER_MODULE (module); */
  /*     PxPacrunnerModuleInterface *iface = PX_PACRUNNER_MODULE_GET_INTERFACE (pacrunner_module); */
  /*     char *pac_ret; */

  /*     iface->set_pac (module, pac); */
  /*     g_debug ("PAC set: %f\n", g_timer_elapsed (timer, NULL)); */
  /*     pac_ret = iface->run (module, uri); */

  /*     ret = g_malloc0 (sizeof (char *) * 2); */
  /*     ret[0] = g_strdup (pac_ret); */
  /*     break; */
  /*   } */
  /* } */
  g_print ("PAC parsed: %f\n", g_timer_elapsed (timer, NULL));
  g_print ("Got: %s\n", pac_data.ret);
  ret = g_malloc0 (sizeof (char *) * 2);
  ret[0] = g_strdup (pac_data.ret);
  return g_steal_pointer (&ret);
}
