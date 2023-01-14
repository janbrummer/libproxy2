#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define PX_TYPE_CONFIG (px_config_get_type ())

G_DECLARE_INTERFACE (PxConfig, px_config, PX, CONFIG, GObject)

struct _PxConfigInterface
{
  GTypeInterface parent_iface;

  gboolean (*is_available) (PxConfig *self);
  char **(*get_config) (PxConfig *self, GUri *uri, GError **error);
};

G_END_DECLS
