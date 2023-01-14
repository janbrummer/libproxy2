#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define PX_TYPE_PACRUNNER (px_pacrunner_get_type ())

G_DECLARE_INTERFACE (PxPacRunner, px_pacrunner, PX, PAC_RUNNER, GObject)

struct _PxPacRunnerInterface
{
  GTypeInterface parent_iface;

  void (*set_pac) (PxPacRunner *pacrunner, const char  *pac);
  char *(*run) (PxPacRunner *self, GUri *uri);
};

G_END_DECLS
