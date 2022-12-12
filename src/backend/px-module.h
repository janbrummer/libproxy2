/* px-module.h
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

#include <gio/gio.h>

#define PX_TYPE_MODULE (px_module_get_type ())

G_DECLARE_FINAL_TYPE (PxModule, px_module, PX, MODULE, GObject)

extern GQuark px_module_error_quark (void);
#define PX_MODULE_ERROR px_module_error_quark ()

typedef enum {
  PX_MODULE_ERROR_INVALID_MODULE = 1001,
  PX_MODULE_ERROR_CONFIGURATION,
} PxModuleErrorCode;

typedef PxModule *(*PxModuleCreate)(void);

PxModule *px_module_create (void);

const char *px_module_get_name (PxModule *self);

PxModule *
px_module_load (GFile               *target,
                GFileInfo           *info,
                gpointer             user_data);
