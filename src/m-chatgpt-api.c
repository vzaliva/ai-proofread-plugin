#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include "m-chatgpt-api.h"

#define CHATGPT_API_URL "https://api.openai.com/v1/chat/completions"

static const gchar *
find_prompt_text(JsonArray *prompts, const gchar *prompt_id)
{
    guint length = json_array_get_length(prompts);
    // Strip "ai-proofread-" prefix from prompt_id
    const gchar *name = prompt_id;
    if (g_str_has_prefix(prompt_id, "ai-proofread-")) {
        name = prompt_id + strlen("ai-proofread-");
    }
    
    for (guint i = 0; i < length; i++) {
        JsonObject *prompt = json_array_get_object_element(prompts, i);
        if (g_strcmp0(json_object_get_string_member(prompt, "name"), name) == 0) {
            return json_object_get_string_member(prompt, "prompt");
        }
    }
    return NULL;
}

gchar *
m_chatgpt_proofread(const gchar *content,
                    const gchar *prompt_id,
                    JsonArray *prompts,
                    const gchar *api_key,
                    GError **error)
{
    SoupSession *session;
    SoupMessage *msg;
    JsonBuilder *builder;
    JsonGenerator *generator;
    JsonNode *root;
    gchar *json_data;
    const gchar *prompt_text;
    gchar *response_text = NULL;
    
    prompt_text = find_prompt_text(prompts, prompt_id);
    if (!prompt_text) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                   "Prompt not found for ID: %s", prompt_id);
        return NULL;
    }

    // Build request JSON
    builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "model");
    json_builder_add_string_value(builder, "gpt-4");
    json_builder_set_member_name(builder, "messages");
    json_builder_begin_array(builder);
    
    // System message with prompt
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "role");
    json_builder_add_string_value(builder, "system");
    json_builder_set_member_name(builder, "content");
    json_builder_add_string_value(builder, prompt_text);
    json_builder_end_object(builder);
    
    // User message with content
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "role");
    json_builder_add_string_value(builder, "user");
    json_builder_set_member_name(builder, "content");
    json_builder_add_string_value(builder, content);
    json_builder_end_object(builder);
    
    json_builder_end_array(builder);
    json_builder_end_object(builder);

    // Generate JSON string
    generator = json_generator_new();
    root = json_builder_get_root(builder);
    json_generator_set_root(generator, root);
    json_data = json_generator_to_data(generator, NULL);

    // Create HTTP session and message
    session = soup_session_new();
    msg = soup_message_new("POST", CHATGPT_API_URL);
    
    // Set headers
    gchar *auth_header = g_strdup_printf("Bearer %s", api_key);
    soup_message_headers_append(soup_message_get_request_headers(msg),
                              "Authorization", auth_header);
    soup_message_headers_append(soup_message_get_request_headers(msg),
                              "Content-Type", "application/json");
    
    // Set request body
    GBytes *request_body = g_bytes_new(json_data, strlen(json_data));
    soup_message_set_request_body_from_bytes(msg, "application/json", request_body);
    g_bytes_unref(request_body);

    // Send request
    GBytes *response = soup_session_send_and_read(session, msg, NULL, error);
    if (response) {
        gsize response_length;
        const gchar *response_data = g_bytes_get_data(response, &response_length);
        
        // Parse response JSON
        JsonParser *parser = json_parser_new();
        if (json_parser_load_from_data(parser, response_data, response_length, error)) {
            JsonNode *root = json_parser_get_root(parser);
            JsonObject *obj = json_node_get_object(root);
            JsonArray *choices = json_object_get_array_member(obj, "choices");
            if (choices && json_array_get_length(choices) > 0) {
                JsonObject *choice = json_array_get_object_element(choices, 0);
                JsonObject *message = json_object_get_object_member(choice, "message");
                response_text = g_strdup(json_object_get_string_member(message, "content"));
            }
        }
        
        g_object_unref(parser);
        g_bytes_unref(response);
    }

    // Cleanup
    g_free(auth_header);
    g_free(json_data);
    g_object_unref(generator);
    json_node_free(root);
    g_object_unref(builder);
    g_object_unref(session);
    g_object_unref(msg);

    return response_text;
} 