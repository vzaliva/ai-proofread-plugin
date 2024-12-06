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

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>
#include <shell/e-shell-view.h>

#include <calendar/gui/e-calendar-view.h>

#include "m-utils.h"
#include "m-calendar-ui.h"

static void
action_calendar_event_cb (GtkAction *action,
			  EShellView *shell_view)
{
	EShellContent *shell_content;
	ECalendarView *calendar_view = NULL;
	GList *selected_events = NULL, *link;

	shell_content = e_shell_view_get_shell_content (shell_view);
	g_object_get (shell_content, "current-view", &calendar_view, NULL);
	if (E_IS_CALENDAR_VIEW (calendar_view)) {
		selected_events = e_calendar_view_get_selected_events (calendar_view);
	}

	g_print ("%s: Has selected events: %d\n", G_STRFUNC, g_list_length (selected_events));

	for (link = selected_events; link; link = g_list_next (link)) {
		ECalendarViewEvent *event = link->data;
		ECalClient *client;
		#if EDS_CHECK_VERSION(3, 33, 2)
		ICalComponent *icalcomp;
		#else
		icalcomponent *icalcomp;
		#endif
		const gchar *summary;

		if (!event || !event->comp_data)
			continue;

		client = event->comp_data->client;
		icalcomp = event->comp_data->icalcomp;

		if (!client || !icalcomp)
			continue;

		#if EDS_CHECK_VERSION(3, 33, 2)
		summary = i_cal_component_get_summary (icalcomp);
		#else
		summary = icalcomponent_get_summary (icalcomp);
		#endif

		g_print ("   Event '%s' from '%s'\n", summary,
			e_source_get_display_name (e_client_get_source (E_CLIENT (client))));
	}

	g_list_free (selected_events);
}

static void
action_calendar_menu_cb (GtkAction *action,
			 EShellView *shell_view)
{
	g_print ("%s: Executed\n", G_STRFUNC);
}

static GtkActionEntry calendar_event_context_entries[] = {
	{ "my-calendar-ui-event-action",
	  "folder-new",
	  N_("M_y Event Action..."),
	  NULL,
	  N_("My Event Action"),
	  G_CALLBACK (action_calendar_event_cb) }
};

static GtkActionEntry calendar_action_menu_entries[] = {
	{ "my-calendar-ui-action",
	  "document-new",
	  N_("M_y Calendar Action..."),
	  NULL,
	  N_("My Calendar Action"),
	  G_CALLBACK (action_calendar_menu_cb) }
};

static void
m_calendar_ui_update_actions_cb (EShellView *shell_view,
				 GtkActionEntry *entries)
{
	EShellWindow *shell_window;
	EShellContent *shell_content;
	ECalendarView *calendar_view = NULL;
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	gboolean has_event = FALSE;

	shell_content = e_shell_view_get_shell_content (shell_view);
	g_object_get (shell_content, "current-view", &calendar_view, NULL);
	if (E_IS_CALENDAR_VIEW (calendar_view)) {
		GList *selected_events = e_calendar_view_get_selected_events (calendar_view);

		has_event = selected_events != NULL;

		g_list_free (selected_events);
	}

	shell_window = e_shell_view_get_shell_window (shell_view);
	ui_manager = e_shell_window_get_ui_manager (shell_window);
	action_group = e_lookup_action_group (ui_manager, "calendar");

	m_utils_enable_actions (action_group, calendar_event_context_entries, G_N_ELEMENTS (calendar_event_context_entries), TRUE);
	m_utils_enable_actions (action_group, calendar_action_menu_entries, G_N_ELEMENTS (calendar_action_menu_entries), has_event);
}

void
m_calendar_ui_init (GtkUIManager *ui_manager,
		    EShellView *shell_view,
		    gchar **ui_definition)
{
	const gchar *ui_def =
		"<menubar name='main-menu'>\n"
		"  <placeholder name='custom-menus'>\n"
		"    <menu action='calendar-actions-menu'>\n"
		"      <menuitem action=\"my-calendar-ui-action\"/>\n"
		"    </menu>\n"
		"  </placeholder>\n"
		"</menubar>\n"
		"\n"
		"<popup name=\"calendar-event-popup\">\n"
		"  <placeholder name=\"event-popup-actions\">\n"
		"    <menuitem action=\"my-calendar-ui-event-action\"/>\n"
		"  </placeholder>\n"
		"</popup>\n";

	EShellWindow *shell_window;
	GtkActionGroup *action_group;

	g_return_if_fail (ui_definition != NULL);

	*ui_definition = g_strdup (ui_def);

	shell_window = e_shell_view_get_shell_window (shell_view);
	action_group = e_shell_window_get_action_group (shell_window, "calendar");

	/* Add actions to the action group. */
	e_action_group_add_actions_localized (
		action_group, GETTEXT_PACKAGE,
		calendar_event_context_entries, G_N_ELEMENTS (calendar_event_context_entries), shell_view);
	e_action_group_add_actions_localized (
		action_group, GETTEXT_PACKAGE,
		calendar_action_menu_entries, G_N_ELEMENTS (calendar_action_menu_entries), shell_view);

	/* Decide whether we want this option to be visible or not */
	g_signal_connect (
		shell_view, "update-actions",
		G_CALLBACK (m_calendar_ui_update_actions_cb),
		shell_view);
}
