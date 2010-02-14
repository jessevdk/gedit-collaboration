/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-manager.h"
#include <gedit/gedit-plugin.h>
#include <libinfinity/adopted/inf-adopted-session.h>
#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-buffer.h>
#include <libinftextgtk/inf-text-gtk-buffer.h>
#include <gedit/gedit-view.h>
#include <libinfinity/common/inf-error.h>
#include "gedit-collaboration.h"

#define GEDIT_COLLABORATION_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_MANAGER, GeditCollaborationManagerPrivate))

struct _GeditCollaborationManagerPrivate
{
	GeditWindow *window;
	InfcNotePlugin note_plugin;

	GHashTable *subscriptions;
};

typedef enum
{
	STATUS_SUBSCRIBE,
	STATUS_SYNCHRONIZING,
	STATUS_JOINING,
	STATUS_SUBSCRIBED
} SubscriptionStatus;

typedef struct
{
	InfcBrowser *browser;
	InfcBrowserIter iter;
	InfcSessionProxy *proxy;

	GeditCollaborationUser *user;
	SubscriptionStatus status;
	GeditView *view;
	GeditTab *tab;

	gint name_failed_counter;
} Subscription;

/* Properties */
enum
{
	PROP_0,
	PROP_WINDOW
};

static void request_join (Subscription *subscription, const gchar *name);

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationManager, gedit_collaboration_manager, G_TYPE_OBJECT)

static void
gedit_collaboration_manager_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_manager_parent_class)->finalize (object);
}

static void
gedit_collaboration_manager_dispose (GObject *object)
{
	GeditCollaborationManager *manager = GEDIT_COLLABORATION_MANAGER (object);

	if (manager->priv->window)
	{
		g_object_unref (manager->priv->window);
		manager->priv->window = NULL;
	}
}

static void
gedit_collaboration_manager_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
	GeditCollaborationManager *self = GEDIT_COLLABORATION_MANAGER (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, self->priv->window);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_manager_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
	GeditCollaborationManager *self = GEDIT_COLLABORATION_MANAGER (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			if (self->priv->window)
			{
				g_object_unref (self->priv->window);
			}

			self->priv->window = g_value_dup_object (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_manager_class_init (GeditCollaborationManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_manager_finalize;
	object_class->dispose = gedit_collaboration_manager_dispose;
	object_class->set_property = gedit_collaboration_manager_set_property;
	object_class->get_property = gedit_collaboration_manager_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_WINDOW,
	                                 g_param_spec_object ("window",
	                                                      "Window",
	                                                      "Window",
	                                                      GEDIT_TYPE_WINDOW,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));


	g_type_class_add_private (object_class, sizeof(GeditCollaborationManagerPrivate));
}

static void
update_saturation_value (GtkWidget        *widget,
                         InfTextGtkBuffer *buffer)
{
	gdouble sat, val;

	gedit_collaboration_get_sv (widget, &sat, &val);
	inf_text_gtk_buffer_set_saturation_value (buffer, sat, val);

}

static void
on_style_set (GtkWidget        *widget,
              GtkStyle         *previous_style,
              InfTextGtkBuffer *buffer)
{
	update_saturation_value (widget, buffer);
}

static InfSession *
create_session_new (InfIo                       *io,
                    InfCommunicationManager     *manager,
                    InfSessionStatus             status,
                    InfCommunicationJoinedGroup *sync_group,
                    InfXmlConnection            *sync_connection,
                    gpointer                     user_data)
{
	GeditCollaborationManager *man = user_data;
	Subscription *subscription;
	InfTextSession *session;
	InfUserTable *user_table;
	InfTextBuffer *buffer;
	GtkTextBuffer *textbuffer;
	GeditTab *tab;

	subscription = g_hash_table_lookup (man->priv->subscriptions,
	                                    sync_connection);

	if (subscription == NULL)
	{
		g_warning ("Could not find subscription!");
		return NULL;
	}

	tab = gedit_window_create_tab (man->priv->window, TRUE);
	subscription->view = gedit_tab_get_view (tab);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (subscription->view), FALSE);

	textbuffer = GTK_TEXT_BUFFER (gedit_tab_get_document (tab));
	user_table = inf_user_table_new();
	buffer = INF_TEXT_BUFFER (inf_text_gtk_buffer_new (textbuffer, user_table));

	update_saturation_value (GTK_WIDGET (gedit_tab_get_view (tab)),
	                         INF_TEXT_GTK_BUFFER (buffer));

	session = inf_text_session_new_with_user_table (manager,
	                                                buffer,
	                                                io,
	                                                user_table,
	                                                status,
	                                                INF_COMMUNICATION_GROUP (sync_group),
	                                                sync_connection);

	g_signal_connect (gedit_tab_get_view (tab),
	                  "style-set",
	                  G_CALLBACK (on_style_set),
	                  buffer);

	subscription->tab = tab;
	return INF_SESSION (session);
}

static void
subscription_free (Subscription *s)
{
	g_object_unref (s->user);

	g_slice_free (Subscription, s);
}

static void
gedit_collaboration_manager_init (GeditCollaborationManager *self)
{
	self->priv = GEDIT_COLLABORATION_MANAGER_GET_PRIVATE (self);

	self->priv->note_plugin.user_data = self;
	self->priv->note_plugin.note_type = "InfText";
	self->priv->note_plugin.session_new = create_session_new;

	self->priv->subscriptions = g_hash_table_new_full (g_direct_hash,
	                                                   g_direct_equal,
	                                                   NULL,
	                                                   (GDestroyNotify)subscription_free);
}

GeditCollaborationManager *
gedit_collaboration_manager_new (GeditWindow *window)
{
	return g_object_new (GEDIT_COLLABORATION_TYPE_MANAGER,
	                     "window", window,
	                     NULL);
}

InfcNotePlugin *
gedit_collaboration_manager_get_note_plugin (GeditCollaborationManager *manager)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_MANAGER (manager), NULL);

	return &(manager->priv->note_plugin);
}

static void
on_join_user_request_finished (InfcUserRequest *request,
                               InfUser         *user,
                               Subscription    *subscription)
{
	InfSession *session;
	InfBuffer *buffer;

	session = infc_session_proxy_get_session (subscription->proxy);
	buffer = inf_session_get_buffer (session);

	/* Time to enable editing */
	subscription->status = STATUS_SUBSCRIBED;

	inf_text_gtk_buffer_set_active_user (INF_TEXT_GTK_BUFFER (buffer),
	                                     INF_TEXT_USER (user));

	gtk_text_view_set_editable (GTK_TEXT_VIEW (subscription->view), TRUE);
}

static void
on_join_user_request_failed (InfcRequest  *request,
                             const GError *error,
                             Subscription *subscription)
{
	if (error->domain == inf_user_error_quark () &&
	    error->code == INF_USER_ERROR_NAME_IN_USE)
	{
		gchar *new_name;
		gchar *suffix;

		++subscription->name_failed_counter;

		suffix = g_strnfill (++subscription->name_failed_counter,
		                     '_');

		new_name = g_strdup_printf ("%s%s",
		                            gedit_collaboration_user_get_name (subscription->user),
		                            suffix);

		request_join (subscription, new_name);

		g_free (suffix);
		g_free (new_name);
	}
	else if (error)
	{
		g_warning ("Join failed: %d, %d, %s", error->code, error->domain, error->message);
	}
}

static void
on_synchronization_failed (InfSession       *session,
                           InfXmlConnection *connection,
                           const GError     *error,
                           Subscription     *subscription)
{
	g_warning ("Synchronization failed: %s", error->message);
}

static void
request_join (Subscription *subscription,
              const gchar  *name)
{
	InfcUserRequest *request;
	InfAdoptedAlgorithm *algorithm;
	GError *error = NULL;
	InfAdoptedStateVector *v;
	gint i;
	gint num_parameters;
	InfSession *session;
	GtkTextBuffer *buffer;
	GtkTextMark *mark;
	GtkTextIter iter;

	GParameter parameters[] = {
		{"vector", {0,}},
		{"name", {0,}},
		{"caret-position", {0,}},
		{"hue", {0,}}
	};

	session = infc_session_proxy_get_session (subscription->proxy);

	num_parameters = sizeof (parameters) / sizeof (GParameter);

	algorithm = inf_adopted_session_get_algorithm (INF_ADOPTED_SESSION (session));

	v = inf_adopted_state_vector_copy(
		inf_adopted_algorithm_get_current(algorithm)
	);

	g_value_init (&parameters[0].value, INF_ADOPTED_TYPE_STATE_VECTOR);
	g_value_take_boxed (&parameters[0].value, v);

	if (name == NULL)
	{
		name = gedit_collaboration_user_get_name (subscription->user);
	}

	g_value_init (&parameters[1].value, G_TYPE_STRING);
	g_value_set_string (&parameters[1].value, name);

	g_value_init (&parameters[2].value, G_TYPE_UINT);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (subscription->view));
	mark = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
	g_value_set_uint (&parameters[2].value, gtk_text_iter_get_offset (&iter));

	g_value_init (&parameters[3].value, G_TYPE_DOUBLE);
	g_value_set_double (&parameters[3].value, gedit_collaboration_user_get_hue (subscription->user));

	request = infc_session_proxy_join_user (subscription->proxy,
	                                        parameters,
	                                        num_parameters,
	                                        &error);

	for (i = 0; i < num_parameters; ++i)
	{
		g_value_unset (&parameters[i].value);
	}

	if (error)
	{
		g_warning ("Oops: %s", error->message);
		g_error_free (error);
		return;
	}

	subscription->status = STATUS_JOINING;

	g_signal_connect_after (request,
	                        "failed",
	                        G_CALLBACK (on_join_user_request_failed),
	                        subscription);

	g_signal_connect_after (request,
	                        "finished",
	                        G_CALLBACK (on_join_user_request_finished),
	                        subscription);
}

static void
on_synchronization_complete (InfSession       *session,
                             InfXmlConnection *connection,
                             Subscription     *subscription)
{
	request_join (subscription, NULL);
}

static void
on_subscribe_request_finished (InfcNodeRequest *request,
                               InfcBrowserIter *iter,
                               Subscription    *subscription)
{
	InfcSessionProxy *proxy;
	InfSession *session;

	proxy = infc_browser_iter_get_session (subscription->browser, iter);
	session = infc_session_proxy_get_session (proxy);

	subscription->status = STATUS_SYNCHRONIZING;
	subscription->proxy = proxy;

	g_signal_connect_after (session,
	                        "synchronization-failed",
	                        G_CALLBACK (on_synchronization_failed),
	                        subscription);

	g_signal_connect_after (session,
	                        "synchronization-complete",
	                        G_CALLBACK (on_synchronization_complete),
	                        subscription);
}

InfcNodeRequest *
gedit_collaboration_manager_subscribe (GeditCollaborationManager *manager,
                                       GeditCollaborationUser    *user,
                                       InfcBrowser               *browser,
                                       const InfcBrowserIter     *iter)
{
	InfcNodeRequest *request;
	Subscription *subscription;
	InfXmlConnection *connection;

	g_return_val_if_fail (GEDIT_COLLABORATION_IS_MANAGER (manager), NULL);
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_USER (user), NULL);
	g_return_val_if_fail (INFC_IS_BROWSER (browser), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	connection = infc_browser_get_connection (browser);

	subscription = g_hash_table_lookup (manager->priv->subscriptions, connection);

	if (subscription)
	{
		gedit_window_set_active_tab (manager->priv->window, subscription->tab);
		return NULL;
	}

	request = infc_browser_iter_subscribe_session (browser, iter);

	subscription = g_slice_new (Subscription);
	subscription->browser = browser;
	subscription->iter = *iter;
	subscription->user = g_object_ref (user);
	subscription->status = STATUS_SUBSCRIBE;
	subscription->name_failed_counter = 0;

	g_hash_table_insert (manager->priv->subscriptions,
	                     connection,
	                     subscription);

	g_signal_connect (request,
	                  "finished",
	                  G_CALLBACK (on_subscribe_request_finished),
	                  subscription);

	return request;
}
