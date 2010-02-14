/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-user.h"
#include "gedit-collaboration.h"

#include <gconf/gconf-client.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define GEDIT_COLLABORATION_USER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_USER, GeditCollaborationUserPrivate))

#define DEFAULT_USER_BASE_KEY "/apps/gedit-2/plugins/collaboration/user"
#define GCONF_CLIENT_DATA_KEY "GeditCollaborationUserGconfClientKey"

static GeditCollaborationUser *default_user = NULL;

struct _GeditCollaborationUserPrivate
{
	gchar *name;
	gdouble hue;

	Gsasl *sasl_context;
	gchar *password;
	gboolean waiting_for_password;
};

/* Properties */
enum
{
	PROP_0,
	PROP_NAME,
	PROP_HUE
};

/* Signals */
enum
{
	REQUEST_PASSWORD,
	NUM_SIGNALS
};

G_DEFINE_TYPE (GeditCollaborationUser, gedit_collaboration_user, G_TYPE_OBJECT)

static guint signals[NUM_SIGNALS] = {0,};

static void
gedit_collaboration_user_finalize (GObject *object)
{
	GeditCollaborationUser *self = GEDIT_COLLABORATION_USER (object);

	g_free (self->priv->name);

	G_OBJECT_CLASS (gedit_collaboration_user_parent_class)->finalize (object);
}

static void
gedit_collaboration_user_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GeditCollaborationUser *self = GEDIT_COLLABORATION_USER (object);

	switch (prop_id)
	{
		case PROP_NAME:
			g_free (self->priv->name);
			self->priv->name = g_value_dup_string (value);
		break;
		case PROP_HUE:
			self->priv->hue = g_value_get_double (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_user_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GeditCollaborationUser *self = GEDIT_COLLABORATION_USER (object);

	switch (prop_id)
	{
		case PROP_NAME:
			g_value_set_string (value, self->priv->name);
		break;
		case PROP_HUE:
			g_value_set_double (value, self->priv->hue);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_user_class_init (GeditCollaborationUserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_user_finalize;
	object_class->set_property = gedit_collaboration_user_set_property;
	object_class->get_property = gedit_collaboration_user_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_NAME,
	                                 g_param_spec_string ("name",
	                                                      "Name",
	                                                      "Name",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_HUE,
	                                 g_param_spec_double ("hue",
	                                                      "Hue",
	                                                      "Hue",
	                                                      0.0,
	                                                      1.0,
	                                                      0,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	signals[REQUEST_PASSWORD] =
		g_signal_new ("request-password",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL,
		              NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE,
		              1,
		              G_TYPE_POINTER);

	g_type_class_add_private (object_class, sizeof(GeditCollaborationUserPrivate));
}

static gchar *
get_password (GeditCollaborationUser *user,
              gpointer                session_data)
{
	gchar *password;

	user->priv->waiting_for_password = TRUE;

	g_free (user->priv->password);
	user->priv->password = NULL;

	g_signal_emit (user, signals[REQUEST_PASSWORD], 0, session_data);
	user->priv->waiting_for_password = FALSE;

	password = user->priv->password;
	user->priv->password = NULL;

	return password;
}

static int
sasl_callback (Gsasl          *context,
               Gsasl_session  *session,
               Gsasl_property  prop)
{
	GeditCollaborationUser *user = gsasl_callback_hook_get (context);
	gpointer session_data = gsasl_session_hook_get (session);

	int rc = GSASL_NO_CALLBACK;

	switch (prop)
	{
		case GSASL_PASSWORD:
		{
			gchar *password = get_password (user,
			                                session_data);

			if (password)
			{
				gsasl_property_set (session, prop, password);
				rc = GSASL_OK;
			}

			g_free (password);
		}
		break;
		case GSASL_AUTHID:
		case GSASL_ANONYMOUS_TOKEN:
			gsasl_property_set (session, prop, user->priv->name);
			rc = GSASL_OK;
		break;
		case GSASL_VALIDATE_ANONYMOUS:
			rc = GSASL_OK;
		break;
		default:
		break;
	}

	return rc;
}

static void
gedit_collaboration_user_init (GeditCollaborationUser *self)
{
	self->priv = GEDIT_COLLABORATION_USER_GET_PRIVATE (self);

	gsasl_init (&self->priv->sasl_context);
	gsasl_callback_set (self->priv->sasl_context, sasl_callback);
	gsasl_callback_hook_set (self->priv->sasl_context, self);
}

GeditCollaborationUser *
gedit_collaboration_user_new (const gchar *name)
{
	return g_object_new (GEDIT_COLLABORATION_TYPE_USER,
	                     "name", name,
	                     NULL);
}

const gchar *
gedit_collaboration_user_get_name (GeditCollaborationUser *user)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_USER (user), NULL);

	return user->priv->name;
}

void
gedit_collaboration_user_set_name (GeditCollaborationUser *user,
                                   const gchar            *name)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_USER (user));
	g_return_if_fail (name != NULL);

	if (g_strcmp0 (user->priv->name, name) != 0)
	{
		g_object_set (user, "name", name, NULL);
	}
}

gdouble
gedit_collaboration_user_get_hue (GeditCollaborationUser *user)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_USER (user), 0);

	return user->priv->hue;
}

void
gedit_collaboration_user_set_hue (GeditCollaborationUser *user,
                                  gdouble                 hue)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_USER (user));

	if (fabs (user->priv->hue - hue) > 1e-7)
	{
		g_object_set (user, "hue", hue, NULL);
	}
}

static void
on_default_user_notify_hue (GeditCollaborationUser *user)
{
	GConfClient *client = g_object_get_data (G_OBJECT (user),
	                                         GCONF_CLIENT_DATA_KEY);

	gconf_client_set_float (client,
	                        DEFAULT_USER_BASE_KEY "/hue",
	                        user->priv->hue,
	                        NULL);
}

static void
on_default_user_notify_name (GeditCollaborationUser *user)
{
	GConfClient *client = g_object_get_data (G_OBJECT (user),
	                                         GCONF_CLIENT_DATA_KEY);

	gconf_client_set_string (client,
	                         DEFAULT_USER_BASE_KEY "/name",
	                         user->priv->name,
	                         NULL);
}

static void
on_default_user_client_notify (GConfClient *client,
                               guint        cnxnid,
                               GConfEntry  *entry)
{
	const gchar *key;
	GConfValue *value;

	key = gconf_entry_get_key (entry);
	value = gconf_entry_get_value (entry);

	if (value == NULL)
	{
		return;
	}

	if (strcmp (entry->key, DEFAULT_USER_BASE_KEY "/name") == 0)
	{
		gedit_collaboration_user_set_name (default_user,
		                                   gconf_value_get_string (value));
	}
	else if (strcmp (entry->key, DEFAULT_USER_BASE_KEY "/hue") == 0)
	{
		gdouble val = gconf_value_get_float (value);
		gboolean isrand = FALSE;

		if (val < 0)
		{
			srand (time (0));
			val = random() / (gdouble)RAND_MAX;

			isrand = TRUE;
		}

		gedit_collaboration_user_set_hue (default_user,
		                                  val);

		if (isrand)
		{
			gconf_client_set_float (client,
			                        DEFAULT_USER_BASE_KEY "/hue",
			                        val,
			                        NULL);
		}
	}
}

GeditCollaborationUser *
gedit_collaboration_user_get_default ()
{
	if (default_user == NULL)
	{
		GConfClient *client = gconf_client_get_default ();
		gdouble hue = -1;
		gchar *name = NULL;
		GConfValue *value;
		gboolean israndom = FALSE;

		value = gconf_client_get (client,
		                          DEFAULT_USER_BASE_KEY "/hue",
		                          NULL);

		if (value != NULL)
		{
			hue = gconf_value_get_float (value);
			gconf_value_free (value);
		}

		if (hue < 0)
		{
			/* Generate random hue */
			israndom = TRUE;
			srand (time (0));

			hue = random () / (double)RAND_MAX;
		}

		value = gconf_client_get (client,
		                          DEFAULT_USER_BASE_KEY "/name",
		                          NULL);

		if (value != NULL)
		{
			name = g_strdup (gconf_value_get_string (value));
			gconf_value_free (value);
		}

		if (!name || !*name)
		{
			g_free (name);
			name = g_strdup (g_get_user_name ());
		}

		default_user = gedit_collaboration_user_new (name);
		gedit_collaboration_user_set_hue (default_user, hue);
		g_free (name);

		g_object_set_data_full (G_OBJECT (default_user),
		                        GCONF_CLIENT_DATA_KEY,
		                        client,
		                        (GDestroyNotify)g_object_unref);

		/* Make sure to save hue to gconf if it was randomly initialized */
		if (israndom)
		{
			on_default_user_notify_hue (default_user);
		}

		gconf_client_add_dir (client,
		                      DEFAULT_USER_BASE_KEY,
		                      GCONF_CLIENT_PRELOAD_ONELEVEL,
		                      NULL);

		gconf_client_notify_add (client,
		                         DEFAULT_USER_BASE_KEY,
		                         (GConfClientNotifyFunc)on_default_user_client_notify,
		                         NULL,
		                         NULL,
		                         NULL);

		g_signal_connect (default_user,
		                  "notify::hue",
		                  G_CALLBACK (on_default_user_notify_hue),
		                  NULL);

		g_signal_connect (default_user,
		                  "notify::name",
		                  G_CALLBACK (on_default_user_notify_name),
		                  NULL);

		g_object_add_weak_pointer (G_OBJECT (default_user),
		                           (gpointer *)&default_user);
	}

	return default_user;
}

Gsasl *
gedit_collaboration_user_get_sasl_context (GeditCollaborationUser *user)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_USER (user), NULL);

	return user->priv->sasl_context;
}

void
gedit_collaboration_user_set_password (GeditCollaborationUser *user,
                                       const gchar            *password)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_USER (user));
	g_return_if_fail (user->priv->waiting_for_password);

	user->priv->password = g_strdup (password);
}
