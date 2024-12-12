#ifndef M_MSG_COMPOSER_EXTENSION_H
#define M_MSG_COMPOSER_EXTENSION_H

#include <glib-object.h>
#include <libebackend/libebackend.h>
#include <gtk/gtk.h>

/* Standard GObject macros */
#define M_TYPE_MSG_COMPOSER_EXTENSION \
	(m_msg_composer_extension_get_type ())
#define M_MSG_COMPOSER_EXTENSION(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), M_TYPE_MSG_COMPOSER_EXTENSION, MMsgComposerExtension))
#define M_MSG_COMPOSER_EXTENSION_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), M_TYPE_MSG_COMPOSER_EXTENSION, MMsgComposerExtensionClass))
#define M_IS_MSG_COMPOSER_EXTENSION(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), M_TYPE_MSG_COMPOSER_EXTENSION))
#define M_IS_MSG_COMPOSER_EXTENSION_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), M_TYPE_MSG_COMPOSER_EXTENSION))
#define M_MSG_COMPOSER_EXTENSION_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), M_TYPE_MSG_COMPOSER_EXTENSION, MMsgComposerExtensionClass))

G_BEGIN_DECLS

typedef struct _MMsgComposerExtension MMsgComposerExtension;
typedef struct _MMsgComposerExtensionClass MMsgComposerExtensionClass;
typedef struct _MMsgComposerExtensionPrivate MMsgComposerExtensionPrivate;

struct _MMsgComposerExtension
{
	EExtension parent;

	MMsgComposerExtensionPrivate *priv;
};

struct _MMsgComposerExtensionClass
{
	EExtensionClass parent;
};

GType	m_msg_composer_extension_get_type	(void);
void	m_msg_composer_extension_type_register	(GTypeModule *type_module);

G_END_DECLS

#endif /* M_MSG_COMPOSER_EXTENSION_H */
