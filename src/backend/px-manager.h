/* px-manager.h
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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define PX_TYPE_MANAGER (px_manager_get_type())
// #define PX_MANAGER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), PX_TYPE_MANAGER, PxManager))

// typedef struct _PxManager PxManager;
// typedef struct _PxManagerClass PxManagerClass;

// struct _PxManagerClass
// {
//   GObjectClass parent_class;

//   gboolean (*get_config) (PxManager *self, GUri *uri);
// };

G_DECLARE_FINAL_TYPE (PxManager, px_manager, PX, MANAGER, GObject)

extern GQuark px_manager_error_quark (void);
#define PX_MANAGER_ERROR px_manager_error_quark ()

typedef enum {
  PX_MANAGER_ERROR_UNKNOWN_METHOD = 1001,
} PxManagerErrorCode;


PxManager *px_manager_new (void);
char **px_manager_get_proxies_sync (PxManager   *self,
                                    const char  *url,
                                    GError     **error);

GBytes *px_manager_pac_download (PxManager  *self,
                                 const char *uri);

char **px_manager_get_configuration (PxManager  *self,
                                     GUri       *uri,
                                     GError    **error);

G_END_DECLS