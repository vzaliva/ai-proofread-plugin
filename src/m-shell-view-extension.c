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
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <shell/e-shell-view.h>

#include "m-mail-ui.h"
#include "m-calendar-ui.h"

#include "m-shell-view-extension.h"

struct _MShellViewExtensionPrivate {
	guint current_ui_id;
	GHashTable *ui_definitions;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (MShellViewExtension, m_shell_view_extension, E_TYPE_EXTENSION, 0,
	G_ADD_PRIVATE_DYNAMIC (MShellViewExtension))

static void
m_shell_extension_get_ui_definition (EShellView *shell_view,
				     const gchar *ui_manager_id,
				     gchar **ui_definition)
{
	EShellWindow *shell_window;
	GtkUIManager *ui_manager;

	g_return_if_fail (shell_view != NULL);
	g_return_if_fail (ui_manager_id != NULL);
	g_return_if_fail (ui_definition != NULL);

	shell_window = e_shell_view_get_shell_window (shell_view);
	ui_manager = e_shell_window_get_ui_manager (shell_window);

	if (g_strcmp0 (ui_manager_id, "org.gnome.evolution.mail") == 0)
		m_mail_ui_init (ui_manager, shell_view, ui_definition);
	else if (g_strcmp0 (ui_manager_id, "org.gnome.evolution.calendars") == 0)
		m_calendar_ui_init (ui_manager, shell_view, ui_definition);
	/* else if (g_strcmp0 (ui_manager_id, "org.gnome.evolution.tasks") == 0)
		m_tasks_ui_init (ui_manager, shell_view, ui_definition);
	else if (g_strcmp0 (ui_manager_id, "org.gnome.evolution.memos") == 0)
		m_memos_ui_init (ui_manager, shell_view, ui_definition);
	else if (g_strcmp0 (ui_manager_id, "org.gnome.evolution.contacts") == 0)
		m_contacts_ui_init (ui_manager, shell_view, ui_definition); */
}

static void
m_shell_view_extension_shell_view_toggled_cb (EShellView *shell_view,
					      MShellViewExtension *shell_view_ext)
{
	EShellViewClass *shell_view_class;
	EShellWindow *shell_window;
	GtkUIManager *ui_manager;
	gpointer key = NULL, value = NULL;
	const gchar *ui_def;
	gboolean is_active, need_update;

	g_return_if_fail (E_IS_SHELL_VIEW (shell_view));
	g_return_if_fail (M_IS_SHELL_VIEW_EXTENSION (shell_view_ext));

	shell_view_class = E_SHELL_VIEW_GET_CLASS (shell_view);
	g_return_if_fail (shell_view_class != NULL);

	shell_window = e_shell_view_get_shell_window (shell_view);
	ui_manager = e_shell_window_get_ui_manager (shell_window);

	need_update = shell_view_ext->priv->current_ui_id != 0;
	if (shell_view_ext->priv->current_ui_id) {
		gtk_ui_manager_remove_ui (ui_manager, shell_view_ext->priv->current_ui_id);
		shell_view_ext->priv->current_ui_id = 0;
	}

	is_active = e_shell_view_is_active (shell_view);
	if (!is_active) {
		if (need_update)
			gtk_ui_manager_ensure_update (ui_manager);

		return;
	}

	if (!g_hash_table_lookup_extended (shell_view_ext->priv->ui_definitions, shell_view_class->ui_manager_id, &key, &value)) {
		gchar *ui_definition = NULL;

		m_shell_extension_get_ui_definition (shell_view, shell_view_class->ui_manager_id, &ui_definition);
		g_hash_table_insert (shell_view_ext->priv->ui_definitions, g_strdup (shell_view_class->ui_manager_id), ui_definition);
	}

	ui_def = g_hash_table_lookup (shell_view_ext->priv->ui_definitions, shell_view_class->ui_manager_id);
	if (ui_def) {
		GError *error = NULL;

		shell_view_ext->priv->current_ui_id = gtk_ui_manager_add_ui_from_string (ui_manager, ui_def, -1, &error);
		need_update = TRUE;

		if (error) {
			g_warning ("%s: Failed to add ui definition: %s", G_STRFUNC, error->message);
			g_error_free (error);
		}
	}

	if (need_update)
		gtk_ui_manager_ensure_update (ui_manager);
}

static void
m_shell_view_extension_constructed (GObject *object)
{
	EExtension *extension;
	EExtensible *extensible;

	/* Chain up to parent's method. */
	G_OBJECT_CLASS (m_shell_view_extension_parent_class)->constructed (object);

	extension = E_EXTENSION (object);
	extensible = e_extension_get_extensible (extension);

	g_signal_connect (E_SHELL_VIEW (extensible), "toggled", G_CALLBACK (m_shell_view_extension_shell_view_toggled_cb), extension);
}

static void
m_shell_view_extension_finalize (GObject *object)
{
	MShellViewExtension *shell_view_ext = M_SHELL_VIEW_EXTENSION (object);

	g_hash_table_destroy (shell_view_ext->priv->ui_definitions);

	/* Chain up to parent's method. */
	G_OBJECT_CLASS (m_shell_view_extension_parent_class)->finalize (object);
}

static void
m_shell_view_extension_class_init (MShellViewExtensionClass *class)
{
	GObjectClass *object_class;
	EExtensionClass *extension_class;

	object_class = G_OBJECT_CLASS (class);
	object_class->constructed = m_shell_view_extension_constructed;
	object_class->finalize = m_shell_view_extension_finalize;

	/* Set the type to extend, it's supposed to implement the EExtensible interface */
	extension_class = E_EXTENSION_CLASS (class);
	extension_class->extensible_type = E_TYPE_SHELL_VIEW;
}

static void
m_shell_view_extension_class_finalize (MShellViewExtensionClass *class)
{
}

static void
m_shell_view_extension_init (MShellViewExtension *shell_view_ext)
{
	shell_view_ext->priv = m_shell_view_extension_get_instance_private (shell_view_ext);
	shell_view_ext->priv->current_ui_id = 0;
	shell_view_ext->priv->ui_definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

void
m_shell_view_extension_type_register (GTypeModule *type_module)
{
	m_shell_view_extension_register_type (type_module);
}
