/* px-pacrunner-module.h
 *
 * Copyright 2022 Jan-Michael Brummer
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

typedef struct _PxPacrunnerModule PxPacrunnerModule;
typedef struct _PxPacrunnerModuleInterface PxPacrunnerModuleInterface;

struct _PxPacrunnerModuleInterface {
  GTypeInterface parent;

  const char *name;
  long version;

  void (*set_pac)(PxModule *px_module, const char *pac);
  char *(*run)(PxModule *px_module, GUri *uri);
};

#define PX_TYPE_PACRUNNER_MODULE (px_pacrunner_module_get_type ())
#define PX_IS_PACRUNNER_MODULE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), PX_TYPE_PACRUNNER_MODULE))
#define PX_PACRUNNER_MODULE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), PX_TYPE_PACRUNNER_MODULE, PxPacrunnerModule))
#define PX_PACRUNNER_MODULE_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE((o), PX_TYPE_PACRUNNER_MODULE, PxPacrunnerModuleInterface))

GType px_pacrunner_module_get_type (void);
