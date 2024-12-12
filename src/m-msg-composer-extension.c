#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include <composer/e-msg-composer.h>
#include <evolution/e-util/e-util.h>

#include "m-msg-composer-extension.h"
#include "m-chatgpt-api.h"

// Temporary debug flag
#define M_MSG_COMPOSER_DEBUG 1

#ifdef M_MSG_COMPOSER_DEBUG
# define DEBUG_MSG(x...) g_print (G_STRLOC ": " x)
#else
# define DEBUG_MSG(x...) g_debug (x)
#endif

struct _MMsgComposerExtensionPrivate {
	JsonArray *prompts;  // Array of prompts loaded from config
	gchar *chatgpt_api_key;     // OpenAI API key
};

struct ProofreadContext {
    EContentEditor *cnt_editor;
    const gchar *prompt_id;
    MMsgComposerExtension *extension;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (MMsgComposerExtension, m_msg_composer_extension, E_TYPE_EXTENSION, 0,
	G_ADD_PRIVATE_DYNAMIC (MMsgComposerExtension))

static JsonArray *
load_prompts(void)
{
	const gchar *config_dir = e_get_user_config_dir();	
	gchar *config_path = g_build_filename(config_dir, "ai-proofread", "prompts.json", NULL);
	JsonParser *parser;
	JsonNode *root;
	JsonArray *prompts = NULL;
	GError *error = NULL;

	DEBUG_MSG("Loading prompts from: %s\n", config_path);

	parser = json_parser_new();
	if (json_parser_load_from_file(parser, config_path, &error)) {
		root = json_parser_get_root(parser);
		if (JSON_NODE_HOLDS_ARRAY(root)) {
			prompts = json_array_ref(json_node_get_array(root));
		}
		DEBUG_MSG("Prompts loaded: %d\n", json_array_get_length(prompts));
	} else {
		DEBUG_MSG("Error loading prompts: %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(parser);
	g_free(config_path);

	return prompts ? prompts : json_array_new();
}

static gchar *
load_api_key(void)
{
    const gchar *home_dir = g_get_home_dir();
    gchar *authinfo_path = g_build_filename(home_dir, ".authinfo", NULL);
    gchar *content = NULL;
    gchar *api_key = NULL;
    GError *error = NULL;

    DEBUG_MSG("Loading authinfo from: %s\n", authinfo_path);

    if (g_file_get_contents(authinfo_path, &content, NULL, &error)) {
        gchar **lines = g_strsplit(content, "\n", -1);
        for (gint i = 0; lines[i] != NULL; i++) {
            // Skip empty lines
            if (lines[i][0] == '\0') continue;

            gchar **tokens = g_strsplit(lines[i], " ", -1);
            gint token_count = g_strv_length(tokens);

            // Look for line matching: machine api.openai.com login apikey password <key>
            if (token_count >= 6 &&
                g_strcmp0(tokens[0], "machine") == 0 &&
                g_strcmp0(tokens[1], "api.openai.com") == 0 &&
                g_strcmp0(tokens[2], "login") == 0 &&
                g_strcmp0(tokens[3], "apikey") == 0 &&
                g_strcmp0(tokens[4], "password") == 0) {
                    api_key = g_strdup(tokens[5]);
                    DEBUG_MSG("Found API key\n");
                    g_strfreev(tokens);
                    break;
            }
            g_strfreev(tokens);
        }
        g_strfreev(lines);
        g_free(content);
    } else {
        DEBUG_MSG("Error loading authinfo: %s\n", error->message);
        g_error_free(error);
    }

    g_free(authinfo_path);

    return api_key;
}

static void
msg_text_cb (GObject *source_object,
             GAsyncResult *result,
             gpointer user_data)
{
    struct ProofreadContext *context = user_data;
    EContentEditor *cnt_editor = context->cnt_editor;
    const gchar *prompt_id = context->prompt_id;
    MMsgComposerExtension *extension = context->extension;
    
    EContentEditorContentHash *content_hash;
    gchar *content, *new_content;
    GError *error = NULL;

    DEBUG_MSG("Getting content finish for prompt: %s\n", prompt_id);
    content_hash = e_content_editor_get_content_finish (cnt_editor, result, &error);
    if (error) {
        DEBUG_MSG("Error getting content: %s\n", error->message);
        g_error_free (error);
        g_free(context);
        return;
    }
    
    if (!content_hash) {
        DEBUG_MSG("No content hash returned\n");
        g_free(context);
        return;
    }

    content = e_content_editor_util_steal_content_data (content_hash, 
        E_CONTENT_EDITOR_GET_TO_SEND_PLAIN, NULL);
    
    if (content) {
        gchar *proofread_text = m_chatgpt_proofread(
            content,
            prompt_id,
            extension->priv->prompts,
            extension->priv->chatgpt_api_key,
            &error
        );

        if (error) {
            DEBUG_MSG("ChatGPT API error: %s\n", error->message);
            
            EMsgComposer *composer = E_MSG_COMPOSER(
                e_extension_get_extensible(E_EXTENSION(extension)));
            
            e_alert_submit(
                E_ALERT_SINK(composer),
                "ai:error-proofreading",
                error->message,
                NULL);
                
            g_error_free(error);
            g_free(context);
            return;
        } else if (proofread_text) {
            new_content = g_strdup(proofread_text);
            g_free(proofread_text);
        } else {
            // Show dialog for no response case too
            EMsgComposer *composer = E_MSG_COMPOSER(
                e_extension_get_extensible(E_EXTENSION(extension)));
            
            GtkWidget *dialog = gtk_message_dialog_new(
                GTK_WINDOW(composer),
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                _("No response received from proofreading service"));
                
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
                
            g_free(context);
            return;
        }

        e_content_editor_insert_content (
            cnt_editor,
            new_content,
            E_CONTENT_EDITOR_INSERT_TEXT_PLAIN | E_CONTENT_EDITOR_INSERT_FROM_PLAIN_TEXT
        );

        g_free(new_content);  // Free new_content after insertion
        g_free(content);
    }

    e_content_editor_util_free_content_hash (content_hash);
    g_free(context);
}

static void
action_msg_composer_prompt_cb (GtkAction *action,
                             MMsgComposerExtension *msg_composer_ext)
{
    const gchar *prompt_id = gtk_action_get_name(action);
    DEBUG_MSG("Action callback triggered for prompt: %s\n", prompt_id);
    
    EMsgComposer *composer;
    EHTMLEditor *editor;    
    EContentEditor *cnt_editor;
    
    g_return_if_fail (M_IS_MSG_COMPOSER_EXTENSION (msg_composer_ext));

    composer = E_MSG_COMPOSER (e_extension_get_extensible (E_EXTENSION (msg_composer_ext)));
    editor = e_msg_composer_get_editor (composer);
    cnt_editor = e_html_editor_get_content_editor (editor);

    // Create context to pass to callback
    struct ProofreadContext *context = g_new(struct ProofreadContext, 1);
    context->cnt_editor = cnt_editor;
    context->prompt_id = prompt_id;
    context->extension = msg_composer_ext;

    DEBUG_MSG("Getting content\n");
    e_content_editor_get_content (
        cnt_editor,
        E_CONTENT_EDITOR_GET_TO_SEND_PLAIN,
        NULL,
        NULL,
        msg_text_cb,
        context
    );    
}

static void
run_button_clicked_cb (GtkButton *button,
                      MMsgComposerExtension *msg_composer_ext)
{
    GtkComboBoxText *combo = g_object_get_data(G_OBJECT(button), "combo");
    const gchar *prompt_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    
    if (prompt_id) {
        DEBUG_MSG("Run button clicked with active selection: %s\n", prompt_id);
        
        EMsgComposer *composer;
        EHTMLEditor *editor;    
        EContentEditor *cnt_editor;
        
        g_return_if_fail (M_IS_MSG_COMPOSER_EXTENSION (msg_composer_ext));

        composer = E_MSG_COMPOSER (e_extension_get_extensible (E_EXTENSION (msg_composer_ext)));
        editor = e_msg_composer_get_editor (composer);
        cnt_editor = e_html_editor_get_content_editor (editor);

        // Create context to pass to callback
        struct ProofreadContext *context = g_new(struct ProofreadContext, 1);
        context->cnt_editor = cnt_editor;
        context->prompt_id = prompt_id;
        context->extension = msg_composer_ext;

        e_content_editor_get_content (
            cnt_editor,
            E_CONTENT_EDITOR_GET_TO_SEND_PLAIN,
            NULL,
            NULL,
            msg_text_cb,
            context
        );
    }
}

static void
m_msg_composer_extension_add_ui (MMsgComposerExtension *msg_composer_ext,
                               EMsgComposer *composer)
{
    g_return_if_fail (M_IS_MSG_COMPOSER_EXTENSION (msg_composer_ext));
    g_return_if_fail (E_IS_MSG_COMPOSER (composer));

    // Check if we have any prompts
    if (!msg_composer_ext->priv->prompts || 
        json_array_get_length(msg_composer_ext->priv->prompts) == 0) {
        DEBUG_MSG("No prompts configured, skipping UI creation\n");
        return;
    }

    if(!msg_composer_ext->priv->chatgpt_api_key) {
        DEBUG_MSG("No API key configured, skipping UI creation\n");
        return;
    }   

    GString *ui_def;
    EHTMLEditor *html_editor;
    GtkActionGroup *action_group;
    GtkUIManager *ui_manager;
    GError *error = NULL;
    guint i, n_prompts;

    html_editor = e_msg_composer_get_editor (composer);
    ui_manager = e_html_editor_get_ui_manager (html_editor);
    action_group = e_html_editor_get_action_group (html_editor, "core");

    // Build UI definition for menu
    ui_def = g_string_new(
        "<menubar name='main-menu'>\n"
        "  <placeholder name='pre-edit-menu'>\n"
        "    <menu action='file-menu'>\n"
        "      <placeholder name='external-editor-holder'>\n"
        "        <menu action='ai-proofread-menu'>\n"
    );

    // Create combo box for toolbar
    GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    
    // Add actions for each prompt
    n_prompts = json_array_get_length(msg_composer_ext->priv->prompts);
    for (i = 0; i < n_prompts; i++) {
        JsonObject *prompt = json_array_get_object_element(msg_composer_ext->priv->prompts, i);
        const gchar *name = json_object_get_string_member(prompt, "name");
        const gchar *prompt_text = json_object_get_string_member(prompt, "prompt");
        
        gchar *action_name = g_strdup_printf("ai-proofread-%s", name);
        GtkActionEntry entry = {
            action_name,
            "tools-check-spelling",
            name,
            NULL,
            prompt_text,
            G_CALLBACK(action_msg_composer_prompt_cb)
        };
        
        gtk_action_group_add_actions(action_group, &entry, 1, msg_composer_ext);
        
        g_string_append_printf(ui_def,
            "          <menuitem action='%s'/>\n",
            action_name);
            
        // Add item to combo box
        gtk_combo_box_text_append(combo, action_name, name);
        
        g_free(action_name);
    }

    // Set initial selection
    if (n_prompts > 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    }

    g_string_append(ui_def,
        "        </menu>\n"
        "      </placeholder>\n"
        "    </menu>\n"
        "  </placeholder>\n"
        "</menubar>\n"
    );

    // Add main menu action
    GtkActionEntry menu_entry = {
        "ai-proofread-menu",
        "tools-check-spelling",
        N_("AI _Proofread"),
        NULL,
        N_("AI Proofread"),
        NULL
    };
    e_action_group_add_actions_localized(action_group, GETTEXT_PACKAGE,
        &menu_entry, 1, msg_composer_ext);

    gtk_ui_manager_add_ui_from_string(ui_manager, ui_def->str, -1, &error);

    if (error) {
        g_warning("%s: Failed to add ui definition: %s", G_STRFUNC, error->message);
        g_error_free(error);
    }

    // Create frame to group controls
    GtkFrame *frame = GTK_FRAME(gtk_frame_new(NULL));
    gtk_frame_set_shadow_type(frame, GTK_SHADOW_IN);  // or GTK_SHADOW_ETCHED_IN for different style
    
    // Create container for combo and button
    GtkBox *hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);  // Add some padding
    
    // Add combo box to container
    gtk_box_pack_start(hbox, GTK_WIDGET(combo), TRUE, TRUE, 0);
    
    // Create and add run button with spell-check icon
    GtkButton *run_button = GTK_BUTTON(gtk_button_new_from_icon_name("tools-check-spelling", GTK_ICON_SIZE_BUTTON));
    gtk_box_pack_start(hbox, GTK_WIDGET(run_button), FALSE, FALSE, 0);
    
    // Store combo reference in button for callback
    g_object_set_data(G_OBJECT(run_button), "combo", combo);
    
    // Add hbox to frame
    gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(hbox));
    
    // Add frame to toolbar
    GtkToolItem *tool_item = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(tool_item), GTK_WIDGET(frame));
    gtk_widget_show_all(GTK_WIDGET(tool_item));
    
    GtkToolbar *toolbar = GTK_TOOLBAR(gtk_ui_manager_get_widget(ui_manager, "/main-toolbar"));
    gtk_toolbar_insert(toolbar, tool_item, -1);

    // Connect signals
    g_signal_connect(run_button, "clicked",
                    G_CALLBACK(run_button_clicked_cb), msg_composer_ext);

    gtk_ui_manager_ensure_update(ui_manager);
    g_string_free(ui_def, TRUE);
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
	msg_composer_ext->priv->prompts = load_prompts();
	msg_composer_ext->priv->chatgpt_api_key = load_api_key();
}

static void
m_msg_composer_extension_dispose (GObject *object)
{
    MMsgComposerExtension *msg_composer_ext = M_MSG_COMPOSER_EXTENSION (object);
    
    if (msg_composer_ext->priv->prompts) {
        json_array_unref(msg_composer_ext->priv->prompts);
        msg_composer_ext->priv->prompts = NULL;
    }

    g_free(msg_composer_ext->priv->chatgpt_api_key);
    msg_composer_ext->priv->chatgpt_api_key = NULL;

    /* Chain up to parent's method */
    G_OBJECT_CLASS (m_msg_composer_extension_parent_class)->dispose (object);
}

void
m_msg_composer_extension_type_register (GTypeModule *type_module)
{
	m_msg_composer_extension_register_type (type_module);
}
