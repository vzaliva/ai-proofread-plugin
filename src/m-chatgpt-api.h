#ifndef M_CHATGPT_API_H
#define M_CHATGPT_API_H

#include <json-glib/json-glib.h>

gchar *m_chatgpt_proofread(const gchar *content, 
                          const gchar *prompt_id,
                          JsonArray *prompts,
                          const gchar *api_key,
                          GError **error);

#endif /* M_CHATGPT_API_H */ 