/*
 * Copyright (C) 2016 Red Hat, Inc. (www.redhat.com)
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef M_UTILS_H
#define M_UTILS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

void		m_utils_enable_actions		(GtkActionGroup *action_group,
						 const GtkActionEntry *entries,
						 guint n_entries,
						 gboolean enable);

G_END_DECLS

#endif /* M_UTILS_H */
