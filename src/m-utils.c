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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

void
m_utils_enable_actions (GtkActionGroup *action_group,
			const GtkActionEntry *entries,
			guint n_entries,
			gboolean enable)
{
	gint ii;

	g_return_if_fail (action_group != NULL);
	g_return_if_fail (entries != NULL);

	for (ii = 0; ii < n_entries; ii++) {
		GtkAction *action;

		action = gtk_action_group_get_action (action_group, entries[ii].name);
		if (!action)
			continue;

		gtk_action_set_sensitive (action, enable);
	}
}
