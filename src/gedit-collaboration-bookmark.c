/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-bookmark.h"
#include <gedit/gedit-plugin.h>
#include "gedit-collaboration.h"

#define GEDIT_COLLABORATION_BOOKMARK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_BOOKMARK, GeditCollaborationBookmarkPrivate))

struct _GeditCollaborationBookmarkPrivate
{
	gchar *name;
	gchar *host;
	gint port;
	GeditCollaborationUser *user;
};

/* Properties */
enum {
	PROP_0,

	PROP_NAME,
	PROP_HOST,
	PROP_PORT,
	PROP_USER
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationBookmark, gedit_collaboration_bookmark, G_TYPE_OBJECT)

static void
gedit_collaboration_bookmark_finalize (GObject *object)
{
	GeditCollaborationBookmark *self = GEDIT_COLLABORATION_BOOKMARK (object);

	g_free (self->priv->name);
	g_free (self->priv->host);
	g_object_unref (self->priv->user);

	G_OBJECT_CLASS (gedit_collaboration_bookmark_parent_class)->finalize (object);
}

static void
gedit_collaboration_bookmark_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
	GeditCollaborationBookmark *self = GEDIT_COLLABORATION_BOOKMARK (object);

	switch (prop_id)
	{
		case PROP_NAME:
			g_free (self->priv->name);
			self->priv->name = g_value_dup_string (value);
		break;
		case PROP_HOST:
			g_free (self->priv->host);
			self->priv->host = g_value_dup_string (value);
		break;
		case PROP_PORT:
			self->priv->port = g_value_get_int (value);

			if (self->priv->port == 0)
			{
				self->priv->port = DEFAULT_INFINOTE_PORT;
			}
		break;
		case PROP_USER:
			if (self->priv->user)
			{
				g_object_unref (self->priv->user);
			}

			self->priv->user = g_value_dup_object (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_bookmark_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
	GeditCollaborationBookmark *self = GEDIT_COLLABORATION_BOOKMARK (object);

	switch (prop_id)
	{
		case PROP_NAME:
			g_value_set_string (value, self->priv->name);
		break;
		case PROP_HOST:
			g_value_set_string (value, self->priv->host);
		break;
		case PROP_PORT:
			g_value_set_int (value, self->priv->port);
		break;
		case PROP_USER:
			g_value_set_object (value, self->priv->user);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_bookmark_constructed (GObject *object)
{
	GeditCollaborationBookmark *self = GEDIT_COLLABORATION_BOOKMARK (object);

	if (self->priv->user == NULL)
	{
		GeditCollaborationUser *user = gedit_collaboration_user_get_default ();

		self->priv->user = gedit_collaboration_user_new (gedit_collaboration_user_get_name (user));
		gedit_collaboration_user_set_hue (self->priv->user,
		                                  gedit_collaboration_user_get_hue (user));
	}
}

static void
gedit_collaboration_bookmark_class_init (GeditCollaborationBookmarkClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_bookmark_finalize;
	object_class->constructed = gedit_collaboration_bookmark_constructed;

	object_class->set_property = gedit_collaboration_bookmark_set_property;
	object_class->get_property = gedit_collaboration_bookmark_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_NAME,
	                                 g_param_spec_string ("name",
	                                                      "Name",
	                                                      "Name",
	                                                      NULL,
	                                                      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
	                                 PROP_HOST,
	                                 g_param_spec_string ("host",
	                                                      "Host",
	                                                      "Host",
	                                                      NULL,
	                                                      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
	                                 PROP_PORT,
	                                 g_param_spec_int ("port",
	                                                   "Port",
	                                                   "Port",
	                                                   0,
	                                                   G_MAXINT,
	                                                   DEFAULT_INFINOTE_PORT,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_USER,
	                                 g_param_spec_object ("user",
	                                                      "User",
	                                                      "User",
	                                                      GEDIT_COLLABORATION_TYPE_USER,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationBookmarkPrivate));
}

static void
gedit_collaboration_bookmark_init (GeditCollaborationBookmark *self)
{
	self->priv = GEDIT_COLLABORATION_BOOKMARK_GET_PRIVATE (self);
}

GeditCollaborationBookmark *
gedit_collaboration_bookmark_new ()
{
	return g_object_new (GEDIT_COLLABORATION_TYPE_BOOKMARK, NULL);
}

const gchar *
gedit_collaboration_bookmark_get_name (GeditCollaborationBookmark *bookmark)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark), NULL);
	return bookmark->priv->name;
}

void
gedit_collaboration_bookmark_set_name (GeditCollaborationBookmark *bookmark,
                                       const gchar                *name)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark));
	g_return_if_fail (name != NULL);

	g_object_set (bookmark, "name", name, NULL);
}

const gchar *
gedit_collaboration_bookmark_get_host (GeditCollaborationBookmark *bookmark)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark), NULL);
	return bookmark->priv->host;
}

void
gedit_collaboration_bookmark_set_host (GeditCollaborationBookmark *bookmark,
                                       const gchar                *host)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark));
	g_return_if_fail (host != NULL);

	g_object_set (bookmark, "host", host, NULL);
}

const gint
gedit_collaboration_bookmark_get_port (GeditCollaborationBookmark *bookmark)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark), 0);
	return bookmark->priv->port;
}

void
gedit_collaboration_bookmark_set_port (GeditCollaborationBookmark *bookmark,
                                       gint                        port)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark));

	g_object_set (bookmark, "port", port, NULL);
}

GeditCollaborationUser *
gedit_collaboration_bookmark_get_user (GeditCollaborationBookmark *bookmark)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_BOOKMARK (bookmark), NULL);
	return bookmark->priv->user;
}
