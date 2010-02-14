/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_DOCUMENT_MESSAGE_H__
#define __GEDIT_COLLABORATION_DOCUMENT_MESSAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE		(gedit_collaboration_document_message_get_type ())
#define GEDIT_COLLABORATION_DOCUMENT_MESSAGE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, GeditCollaborationDocumentMessage))
#define GEDIT_COLLABORATION_DOCUMENT_MESSAGE_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, GeditCollaborationDocumentMessage const))
#define GEDIT_COLLABORATION_DOCUMENT_MESSAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, GeditCollaborationDocumentMessageClass))
#define GEDIT_COLLABORATION_IS_DOCUMENT_MESSAGE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE))
#define GEDIT_COLLABORATION_IS_DOCUMENT_MESSAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE))
#define GEDIT_COLLABORATION_DOCUMENT_MESSAGE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, GeditCollaborationDocumentMessageClass))

typedef struct _GeditCollaborationDocumentMessage		GeditCollaborationDocumentMessage;
typedef struct _GeditCollaborationDocumentMessageClass		GeditCollaborationDocumentMessageClass;
typedef struct _GeditCollaborationDocumentMessagePrivate	GeditCollaborationDocumentMessagePrivate;

struct _GeditCollaborationDocumentMessage {
	GtkInfoBar parent;

	GeditCollaborationDocumentMessagePrivate *priv;
};

struct _GeditCollaborationDocumentMessageClass {
	GtkInfoBarClass parent_class;
};

GType gedit_collaboration_document_message_get_type (void) G_GNUC_CONST;
GType gedit_collaboration_document_message_register_type (GTypeModule *type_module);

GQuark gedit_collaboration_document_message_error_quark (void);

GtkWidget *gedit_collaboration_document_message_new_error (const GError *error);
GtkWidget *gedit_collaboration_document_message_new_progress (const gchar *message);

void gedit_collaboration_document_message_update (GeditCollaborationDocumentMessage *document_message,
                                                  gdouble                            fraction);

gchar *gedit_collaboration_document_message_error_string (const GError *error);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_DOCUMENT_MESSAGE_H__ */
