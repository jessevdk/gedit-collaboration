/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-user-store.h"
#include <gedit/gedit-plugin.h>

#define GEDIT_COLLABORATION_USER_STORE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_USER_STORE, GeditCollaborationUserStorePrivate))

struct _GeditCollaborationUserStorePrivate
{
	InfUserTable *user_table;
	gboolean show_unavailable;
};

enum
{
	PROP_0,
	PROP_USER_TABLE,
	PROP_SHOW_UNAVAILABLE
};

static void remove_user (GeditCollaborationUserStore *store, InfUser *user, gboolean disconnect_status);
static void add_user (GeditCollaborationUserStore *store, InfUser *user);

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationUserStore, gedit_collaboration_user_store, GTK_TYPE_LIST_STORE)

static void
gedit_collaboration_user_store_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_user_store_parent_class)->finalize (object);
}

static gboolean
find_user (GeditCollaborationUserStore *store,
           InfUser                     *user,
           GtkTreeIter                 *iter)
{
	GtkTreeIter found;
	InfUser *other;

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &found))
	{
		return FALSE;
	}

	do
	{
		gtk_tree_model_get (GTK_TREE_MODEL (store),
		                    &found,
		                    GEDIT_COLLABORATION_USER_STORE_COLUMN_USER,
		                    &other,
		                    -1);

		if (user == other)
		{
			g_object_unref (other);
			*iter = found;

			return TRUE;
		}

		g_object_unref (other);
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &found));

	return FALSE;
}

static void
user_changed (GeditCollaborationUserStore *store,
              InfUser                     *user)
{
	GtkTreeIter iter;

	if (find_user (store, user, &iter))
	{
		GtkTreePath *path;

		path = gtk_tree_model_get_path (GTK_TREE_MODEL (store),
		                                &iter);

		gtk_tree_model_row_changed (GTK_TREE_MODEL (store),
		                            path,
		                            &iter);
		gtk_tree_path_free (path);
	}
	else
	{
		add_user (store, user);
	}
}

static void
on_user_notify (InfUser                     *user,
                GParamSpec                  *spec,
                GeditCollaborationUserStore *store)
{
	const gchar *name = g_param_spec_get_name (spec);

	if (g_strcmp0 (name, "name") == 0 ||
	    g_strcmp0 (name, "hue") == 0)
	{
		user_changed (store, user);
	}
	else if (g_strcmp0 (name, "status") == 0)
	{
		if (!store->priv->show_unavailable)
		{
			if (inf_user_get_status (user) == INF_USER_UNAVAILABLE)
			{
				remove_user (store, user, FALSE);
			}
			else
			{
				user_changed (store, user);
			}
		}
		else
		{
			user_changed (store, user);
		}
	}
}

static void
remove_user (GeditCollaborationUserStore *store,
             InfUser                     *user,
             gboolean                     disconnect_status)
{
	GtkTreeIter iter;

	if (find_user (store, user, &iter))
	{
		if (disconnect_status)
		{
			g_signal_handlers_disconnect_by_func (user,
			                                      G_CALLBACK (on_user_notify),
			                                      store);
		}

		gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
	}
}

static void
add_user (GeditCollaborationUserStore *store,
          InfUser                     *user)
{
	GtkTreeIter iter;

	if (store->priv->show_unavailable ||
	    inf_user_get_status (user) != INF_USER_UNAVAILABLE)
	{
		gtk_list_store_append (GTK_LIST_STORE (store),
			               &iter);

		gtk_list_store_set (GTK_LIST_STORE (store),
			            &iter,
			            GEDIT_COLLABORATION_USER_STORE_COLUMN_USER,
			            user,
			            -1);
	}

	g_signal_connect (user,
	                  "notify",
	                  G_CALLBACK (on_user_notify),
	                  store);
}

static void
on_add_user (InfUserTable                *table,
             InfUser                     *user,
             GeditCollaborationUserStore *store)
{
	add_user (store, user);
}

static void
on_remove_user (InfUserTable                *table,
                InfUser                     *user,
                GeditCollaborationUserStore *store)
{
	remove_user (store, user, TRUE);
}

static void
disconnect_user (InfUser  *user,
                 gpointer  data)
{
	g_signal_handlers_disconnect_by_func (user,
	                                      G_CALLBACK (on_user_notify),
	                                      data);
}

static void
gedit_collaboration_user_table_dispose (GObject *object)
{
	GeditCollaborationUserStore *store = GEDIT_COLLABORATION_USER_STORE (object);

	if (store->priv->user_table)
	{
		g_signal_handlers_disconnect_by_func (store->priv->user_table,
		                                      G_CALLBACK (on_add_user),
		                                      store);

		g_signal_handlers_disconnect_by_func (store->priv->user_table,
		                                      G_CALLBACK (on_remove_user),
		                                      store);

		inf_user_table_foreach_user (store->priv->user_table,
		                             disconnect_user,
		                             store);

		g_object_unref (store->priv->user_table);
		store->priv->user_table = NULL;
	}
}

static void
refresh (GeditCollaborationUserStore *store)
{

}

static void
gedit_collaboration_user_store_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
	GeditCollaborationUserStore *self = GEDIT_COLLABORATION_USER_STORE (object);
	
	switch (prop_id)
	{
		case PROP_USER_TABLE:
			if (self->priv->user_table)
			{
				g_object_unref (self->priv->user_table);
			}

			self->priv->user_table = g_value_dup_object (value);

			refresh (self);
		break;
		case PROP_SHOW_UNAVAILABLE:
			self->priv->show_unavailable = g_value_get_boolean (value);

			refresh (self);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_user_store_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GeditCollaborationUserStore *self = GEDIT_COLLABORATION_USER_STORE (object);
	
	switch (prop_id)
	{
		case PROP_USER_TABLE:
			g_value_set_object (value, self->priv->user_table);
		break;
		case PROP_SHOW_UNAVAILABLE:
			g_value_set_boolean (value, self->priv->show_unavailable);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
foreach_add_user (InfUser                     *user,
                  GeditCollaborationUserStore *store)
{
	add_user (store, user);
}

static void
gedit_collaboration_user_store_constructed (GObject *object)
{
	GeditCollaborationUserStore *store = GEDIT_COLLABORATION_USER_STORE (object);

	if (!store->priv->user_table)
	{
		return;
	}

	inf_user_table_foreach_user (store->priv->user_table,
	                             (InfUserTableForeachUserFunc)foreach_add_user,
	                             store);

	g_signal_connect (store->priv->user_table,
	                  "add-user",
	                  G_CALLBACK (on_add_user),
	                  store);

	g_signal_connect (store->priv->user_table,
	                  "remove-user",
	                  G_CALLBACK (on_remove_user),
	                  store);
}

static void
gedit_collaboration_user_store_class_init (GeditCollaborationUserStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_user_store_finalize;
	object_class->dispose = gedit_collaboration_user_table_dispose;

	object_class->set_property = gedit_collaboration_user_store_set_property;
	object_class->get_property = gedit_collaboration_user_store_get_property;

	object_class->constructed = gedit_collaboration_user_store_constructed;

	g_object_class_install_property (object_class,
	                                 PROP_USER_TABLE,
	                                 g_param_spec_object ("user-table",
	                                                      "User Table",
	                                                      "User table",
	                                                      INF_TYPE_USER_TABLE,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_SHOW_UNAVAILABLE,
	                                 g_param_spec_boolean ("show-unavailable",
	                                                       "Show Unavailable",
	                                                       "Show unavailable",
	                                                       TRUE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationUserStorePrivate));
}

static gint
iter_compare_func (GtkTreeModel *model,
                   GtkTreeIter  *a,
                   GtkTreeIter  *b,
                   gpointer      data)
{
	InfUser *auser;
	InfUser *buser;
	gchar *aname;
	gchar *bname;
	gint ret;

	gtk_tree_model_get (model,
	                    a,
	                    GEDIT_COLLABORATION_USER_STORE_COLUMN_USER,
	                    &auser,
	                    -1);

	gtk_tree_model_get (model,
	                    b,
	                    GEDIT_COLLABORATION_USER_STORE_COLUMN_USER,
	                    &buser,
	                    -1);

	aname = g_utf8_casefold (inf_user_get_name (auser), -1);
	bname = g_utf8_casefold (inf_user_get_name (buser), -1);

	ret = g_utf8_collate (aname, bname);

	g_free (aname);
	g_free (bname);

	g_object_unref (auser);
	g_object_unref (buser);

	return ret;
}

static void
gedit_collaboration_user_store_init (GeditCollaborationUserStore *self)
{
	GType column_types[] = {
		INF_TYPE_USER
	};

	self->priv = GEDIT_COLLABORATION_USER_STORE_GET_PRIVATE (self);

	gtk_list_store_set_column_types (GTK_LIST_STORE (self),
	                                 sizeof (column_types) / sizeof (GType),
	                                 column_types);

	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (self),
	                                         iter_compare_func,
	                                         NULL,
	                                         NULL);
}

GeditCollaborationUserStore *
gedit_collaboration_user_store_new (InfUserTable *user_table,
                                    gboolean      show_unavailable)
{
	return g_object_new (GEDIT_COLLABORATION_TYPE_USER_STORE,
	                     "user-table", user_table,
	                     "show-unavailable", show_unavailable,
	                     NULL);
}

InfUser *
gedit_collaboration_user_store_get_user (GeditCollaborationUserStore *store,
                                         GtkTreeIter                 *iter)
{
	InfUser *user;

	g_return_val_if_fail (GEDIT_COLLABORATION_IS_USER_STORE (store), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	gtk_tree_model_get (GTK_TREE_MODEL (store),
	                    iter,
	                    GEDIT_COLLABORATION_USER_STORE_COLUMN_USER,
	                    &user,
	                    -1);

	return user;
}
