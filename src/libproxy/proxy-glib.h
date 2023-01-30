/* proxy-glib.h
 *
 * Copyright 2022-2023 The Libproxy Team
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

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <gio/gio.h>

#include <proxy.h>

void
px_proxy_factory_get_proxies_async (pxProxyFactory      *self,
                                    const char          *url,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data);

char **
px_proxy_factory_get_proxies_async_finish (pxProxyFactory  *self,
                                           GAsyncResult    *result,
                                           GError         **error);

#ifdef __cplusplus
}
#endif
