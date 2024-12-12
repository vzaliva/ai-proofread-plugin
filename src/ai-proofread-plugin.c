
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>

#include "m-msg-composer-extension.h"

#define AI_PROOFREAD_VERSION "1.0.0"

/* Module Entry Points */
void e_module_load (GTypeModule *type_module);
void e_module_unload (GTypeModule *type_module);

G_MODULE_EXPORT void
e_module_load (GTypeModule *type_module)
{
	g_info("Loading AI Proofread Plugin v%s", AI_PROOFREAD_VERSION);
	m_msg_composer_extension_type_register (type_module);
}

G_MODULE_EXPORT void
e_module_unload (GTypeModule *type_module)
{
}
