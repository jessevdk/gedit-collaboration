/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_USER_STORE_H__
#define __GEDIT_COLLABORATION_USER_STORE_H__

#include <gtk/gtk.h>
#include <libinfinity/common/inf-user-table.h>

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_USER_STORE		(gedit_collaboration_user_store_get_type ())
#define GEDIT_COLLABORATION_USER_STORE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_USER_STORE, GeditCollaborationUserStore))
#define GEDIT_COLLABORATION_USER_STORE_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_USER_STORE, GeditCollaborationUserStore const))
#define GEDIT_COLLABORATION_USER_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_USER_STORE, GeditCollaborationUserStoreClass))
#define GEDIT_COLLABORATION_IS_USER_STORE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_USER_STORE))
#define GEDIT_COLLABORATION_IS_USER_STORE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_USER_STORE))
#define GEDIT_COLLABORATION_USER_STORE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_USER_STORE, GeditCollaborationUserStoreClass))

typedef struct _GeditCollaborationUserStore		GeditCollaborationUserStore;
typedef struct _GeditCollaborationUserStoreClass	GeditCollaborationUserStoreClass;
typedef struct _GeditCollaborationUserStorePrivate	GeditCollaborationUserStorePrivate;

typedef enum
{
	GEDIT_COLLABORATION_USER_STORE_COLUMN_USER
} GeditCollaborationUserStoreColumn;

struct _GeditCollaborationUserStore
{
	GtkListStore parent;

	GeditCollaborationUserStorePrivate *priv;
};

struct _GeditCollaborationUserStoreClass
{
	GtkListStoreClass parent_class;
};

GType gedit_collaboration_user_store_get_type (void) G_GNUC_CONST;
void _gedit_collaboration_user_store_register_type (GTypeModule *type_module);

GeditCollaborationUserStore *gedit_collaboration_user_store_new (InfUserTable *user_table,
                                                                 gboolean      show_unavailable);
InfUser *gedit_collaboration_user_store_get_user (GeditCollaborationUserStore *store,
                                                  GtkTreeIter                 *iter);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_USER_STORE_H__ */
