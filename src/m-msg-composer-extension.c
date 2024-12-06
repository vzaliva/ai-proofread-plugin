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

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <composer/e-msg-composer.h>

#include "m-msg-composer-extension.h"

struct _MMsgComposerExtensionPrivate {
	gint dummy;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (MMsgComposerExtension, m_msg_composer_extension, E_TYPE_EXTENSION, 0,
	G_ADD_PRIVATE_DYNAMIC (MMsgComposerExtension))

static void
action_msg_composer_cb (GtkAction *action,
			MMsgComposerExtension *msg_composer_ext)
{
	EMsgComposer *composer;

	g_return_if_fail (M_IS_MSG_COMPOSER_EXTENSION (msg_composer_ext));

	composer = E_MSG_COMPOSER (e_extension_get_extensible (E_EXTENSION (msg_composer_ext)));

	g_print ("%s: for composer '%s'\n", G_STRFUNC, gtk_window_get_title (GTK_WINDOW (composer)));
}

static GtkActionEntry msg_composer_entries[] = {
	{ "my-msg-composer-action",
	  "document-new",
	  N_("M_y Message Composer Action..."),
	  NULL,
	  N_("My Message Composer Action"),
	  G_CALLBACK (action_msg_composer_cb) }
};

static void
m_msg_composer_extension_add_ui (MMsgComposerExtension *msg_composer_ext,
				 EMsgComposer *composer)
{
	const gchar *ui_def =
		"<menubar name='main-menu'>\n"
		"  <placeholder name='pre-edit-menu'>\n"
		"    <menu action='file-menu'>\n"
		"      <placeholder name='external-editor-holder'>\n"
		"        <menuitem action='my-msg-composer-action'/>\n"
		"      </placeholder>\n"
		"    </menu>\n"
		"  </placeholder>\n"
		"</menubar>\n"
		"\n"
		"<toolbar name='main-toolbar'>\n"
		"  <toolitem action='my-msg-composer-action'/>\n"
		"</toolbar>\n";

	EHTMLEditor *html_editor;
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GError *error = NULL;

	g_return_if_fail (M_IS_MSG_COMPOSER_EXTENSION (msg_composer_ext));
	g_return_if_fail (E_IS_MSG_COMPOSER (composer));

	html_editor = e_msg_composer_get_editor (composer);
	ui_manager = e_html_editor_get_ui_manager (html_editor);
	action_group = e_html_editor_get_action_group (html_editor, "core");

	/* Add actions to the action group. */
	e_action_group_add_actions_localized (
		action_group, GETTEXT_PACKAGE,
		msg_composer_entries, G_N_ELEMENTS (msg_composer_entries), msg_composer_ext);

	gtk_ui_manager_add_ui_from_string (ui_manager, ui_def, -1, &error);

	if (error) {
		g_warning ("%s: Failed to add ui definition: %s", G_STRFUNC, error->message);
		g_error_free (error);
	}

	gtk_ui_manager_ensure_update (ui_manager);
}

static void
m_msg_composer_extension_constructed (GObject *object)
{
	EExtension *extension;
	EExtensible *extensible;

	/* Chain up to parent's method. */
	G_OBJECT_CLASS (m_msg_composer_extension_parent_class)->constructed (object);

	extension = E_EXTENSION (object);
	extensible = e_extension_get_extensible (extension);

	m_msg_composer_extension_add_ui (M_MSG_COMPOSER_EXTENSION (object), E_MSG_COMPOSER (extensible));
}

static void
m_msg_composer_extension_class_init (MMsgComposerExtensionClass *class)
{
	GObjectClass *object_class;
	EExtensionClass *extension_class;

	object_class = G_OBJECT_CLASS (class);
	object_class->constructed = m_msg_composer_extension_constructed;

	/* Set the type to extend, it's supposed to implement the EExtensible interface */
	extension_class = E_EXTENSION_CLASS (class);
	extension_class->extensible_type = E_TYPE_MSG_COMPOSER;
}

static void
m_msg_composer_extension_class_finalize (MMsgComposerExtensionClass *class)
{
}

static void
m_msg_composer_extension_init (MMsgComposerExtension *msg_composer_ext)
{
	msg_composer_ext->priv = m_msg_composer_extension_get_instance_private (msg_composer_ext);
}

void
m_msg_composer_extension_type_register (GTypeModule *type_module)
{
	m_msg_composer_extension_register_type (type_module);
}
