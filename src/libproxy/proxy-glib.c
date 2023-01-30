/* proxy-glib.c
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

#include <glib.h>

#include "proxy-glib.h"

static void
get_proxies_thread (GTask        *task,
                    gpointer      source_object,
                    gpointer      task_data,
                    GCancellable *cancellable)
{
  pxProxyFactory *self = source_object;
  char *uri = g_task_get_task_data (task);
  char **ret = px_proxy_factory_get_proxies (self, uri);

  g_task_return_pointer (task, ret, (GDestroyNotify)g_strfreev);
}

char **
px_proxy_factory_get_proxies_async_finish (pxProxyFactory  *self,
                                           GAsyncResult    *result,
                                           GError         **error)
{
  return g_task_propagate_pointer (G_TASK (result), error);
}

void
px_proxy_factory_get_proxies_async (pxProxyFactory      *self,
                                    const char          *url,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data)
{
  GTask *task;

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, px_proxy_factory_get_proxies_async);

  g_task_set_task_data (task, g_strdup (url), (GDestroyNotify)g_free);

  g_task_set_return_on_cancel (task, TRUE);

  g_task_run_in_thread (task, get_proxies_thread);
}
