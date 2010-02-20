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
#include "gedit-collaboration-document-message.h"
#include "gedit-collaboration-undo-manager.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#define SESSION_TAB_DATA_KEY "GeditCollaborationManagerSessionTabDataKey"
#define TAB_SUBSCRIPTION_DATA_KEY "GeditCollaborationManagerTabSubscriptionDataKey"

#define GEDIT_COLLABORATION_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_MANAGER, GeditCollaborationManagerPrivate))

struct _GeditCollaborationManagerPrivate
{
	GeditWindow *window;
	InfcNotePlugin note_plugin;

	GSList *subscriptions;
	GHashTable *subscription_map;
};

enum
{
	SYNCHRONIZATION_COMPLETE,
	SYNCHRONIZATION_PROGRESS,
	SYNCHRONIZATION_FAILED,
	STYLE_SET,
	VIEW_DESTROYED,
	SESSION_CLOSE,
	CONNECTION_STATUS,
	NUM_EXTERNAL_SIGNALS
};

typedef struct
{
	InfcBrowser *browser;
	InfcBrowserIter iter;
	InfcSessionProxy *proxy;

	GeditCollaborationUser *user;
	GeditTab *tab;
	GeditCollaborationManager *manager;

	gint name_failed_counter;
	gulong signal_handlers[NUM_EXTERNAL_SIGNALS];

	GTimer *progress_timer;
	gdouble progress_start;
	GtkWidget *progress_area;

	gboolean loading;
} Subscription;

/* Properties */
enum
{
	PROP_0,
	PROP_WINDOW
};

static void request_join (Subscription *subscription, const gchar *name);
static void subscription_free (Subscription *subscription);

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

		g_hash_table_destroy (manager->priv->subscription_map);

		g_slist_foreach (manager->priv->subscriptions,
		                 (GFunc)subscription_free,
		                 NULL);

		g_slist_free (manager->priv->subscriptions);
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

static void
subscription_free (Subscription *subscription)
{
	if (subscription->tab)
	{
		gedit_collaboration_manager_clear_colors (subscription->manager,
		                                          subscription->tab);
	}

	if (subscription->proxy != NULL)
	{
		InfXmlConnection *connection;
		InfSession *session = infc_session_proxy_get_session (subscription->proxy);

		gint handlers[] = {
			SYNCHRONIZATION_COMPLETE,
			SYNCHRONIZATION_PROGRESS,
			SYNCHRONIZATION_FAILED,
			SESSION_CLOSE
		};

		gint i;

		for (i = 0; i < sizeof (handlers) / sizeof (gint); ++i)
		{
			if (subscription->signal_handlers[handlers[i]] != 0)
			{
				g_signal_handler_disconnect (session,
				                             subscription->signal_handlers[handlers[i]]);
			}
		}

		connection = infc_session_proxy_get_connection (subscription->proxy);

		if (connection != NULL)
		{
			g_signal_handler_disconnect (connection,
			                             subscription->signal_handlers[CONNECTION_STATUS]);
		}

		/* Close the session */
		if (inf_session_get_status (session) != INF_SESSION_CLOSED)
		{
			inf_session_close (infc_session_proxy_get_session (subscription->proxy));
		}
	}

	if (subscription->tab != NULL &&
	    subscription->signal_handlers[STYLE_SET] != 0)
	{
		g_signal_handler_disconnect (gedit_tab_get_view (subscription->tab),
		                             subscription->signal_handlers[STYLE_SET]);
	}

	if (subscription->tab != NULL)
	{
		GeditDocument *doc;

		doc = gedit_tab_get_document (subscription->tab);

		g_object_set_data (G_OBJECT (subscription->tab),
		                   TAB_SUBSCRIPTION_DATA_KEY,
		                   NULL);

		g_signal_handler_disconnect (gedit_tab_get_view (subscription->tab),
		                             subscription->signal_handlers[VIEW_DESTROYED]);

		/* Reset the undo manager */
		gtk_source_buffer_set_undo_manager (GTK_SOURCE_BUFFER (doc),
		                                    NULL);

		if (subscription->loading)
		{
			gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (doc));
			gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (doc));
		}
	}

	if (subscription->user != NULL)
	{
		g_object_unref (subscription->user);
	}

	if (subscription->progress_timer != NULL)
	{
		g_timer_destroy (subscription->progress_timer);
	}

	if (subscription->browser != NULL)
	{
		g_object_unref (subscription->browser);
	}

	g_slice_free (Subscription, subscription);
}

static void
close_subscription (Subscription *subscription)
{
	GObject *proxy;

	proxy = g_object_ref (subscription->proxy);

	g_hash_table_remove (subscription->manager->priv->subscription_map,
	                     subscription->proxy);

	subscription->manager->priv->subscriptions =
		g_slist_remove (subscription->manager->priv->subscriptions,
		                subscription);

	subscription_free (subscription);
	g_object_unref (proxy);
}

static void
on_view_destroyed (GtkWidget    *tab,
                   Subscription *subscription)
{
	close_subscription (subscription);
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
	InfTextSession *session;
	InfUserTable *user_table;
	InfTextBuffer *buffer;
	GtkTextBuffer *textbuffer;
	GeditTab *tab;
	GdkCursor *cursor;
	GeditView *view;

	tab = gedit_window_create_tab (man->priv->window, TRUE);
	view = gedit_tab_get_view (tab);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);

	cursor = gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (view)),
	                                     GDK_WATCH);
	gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (view)),
	                       cursor);
	gdk_cursor_unref (cursor);

	textbuffer = GTK_TEXT_BUFFER (gedit_tab_get_document (tab));
	user_table = inf_user_table_new ();
	buffer = INF_TEXT_BUFFER (inf_text_gtk_buffer_new (textbuffer, user_table));

	update_saturation_value (GTK_WIDGET (view),
	                         INF_TEXT_GTK_BUFFER (buffer));

	session = inf_text_session_new_with_user_table (manager,
	                                                buffer,
	                                                io,
	                                                user_table,
	                                                status,
	                                                INF_COMMUNICATION_GROUP (sync_group),
	                                                sync_connection);

	g_object_unref (buffer);
	g_object_unref (user_table);

	g_object_set_data (G_OBJECT (session),
	                   SESSION_TAB_DATA_KEY,
	                   tab);

	return INF_SESSION (session);
}

static void
gedit_collaboration_manager_init (GeditCollaborationManager *self)
{
	self->priv = GEDIT_COLLABORATION_MANAGER_GET_PRIVATE (self);

	self->priv->note_plugin.user_data = self;
	self->priv->note_plugin.note_type = "InfText";
	self->priv->note_plugin.session_new = create_session_new;

	self->priv->subscription_map = g_hash_table_new_full (g_direct_hash,
	                                                      g_direct_equal,
	                                                      (GDestroyNotify)g_object_unref,
	                                                      NULL);
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
	GeditView *view;
	GeditDocument *doc;
	GeditCollaborationUndoManager *undo_manager;

	session = infc_session_proxy_get_session (subscription->proxy);
	buffer = inf_session_get_buffer (session);

	/* Time to enable editing */
	inf_text_gtk_buffer_set_active_user (INF_TEXT_GTK_BUFFER (buffer),
	                                     INF_TEXT_USER (user));

	view = gedit_tab_get_view (subscription->tab);
	doc = gedit_tab_get_document (subscription->tab);

	gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (doc));
	gtk_source_buffer_end_not_undoable_action (GTK_SOURCE_BUFFER (doc));
	subscription->loading = FALSE;

	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), FALSE);

	/* Install the special undo manager */
	undo_manager = gedit_collaboration_undo_manager_new (INF_ADOPTED_SESSION (session),
	                                                     INF_ADOPTED_USER (user));

	gtk_source_buffer_set_undo_manager (GTK_SOURCE_BUFFER (doc),
	                                    GTK_SOURCE_UNDO_MANAGER (undo_manager));

	g_object_unref (undo_manager);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), TRUE);
	gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (view)),
	                       NULL);
}

static void
handle_error (Subscription *subscription,
              const GError *error)
{
	/* Show the error nicely in the document, and cancel the session,
	   cleanup, etc */
#ifndef GEDIT_STABLE
	if (subscription->tab)
	{
		GtkWidget *message_area;

		message_area = gedit_collaboration_document_message_new_error (error);
		gtk_widget_show (message_area);
		gedit_tab_set_info_bar (subscription->tab, message_area);

		g_signal_connect (message_area,
		                  "response",
		                  G_CALLBACK (gtk_widget_destroy),
		                  NULL);

		g_signal_connect (message_area,
		                  "close",
		                  G_CALLBACK (gtk_widget_destroy),
		                  NULL);
	}
	else
	{
#endif
		gchar *primary;
		gchar *escaped;
		gchar *secondary;
		GtkWidget *dialog;

		primary = gedit_collaboration_document_message_error_string (error);
		escaped = g_markup_escape_text (primary, -1);
		secondary = g_markup_escape_text (error->message, -1);

		dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (subscription->manager->priv->window),
		                                             GTK_DIALOG_DESTROY_WITH_PARENT,
		                                             GTK_MESSAGE_ERROR,
		                                             GTK_BUTTONS_CLOSE,
		                                             "<b>%s</b>\n\n<small>%s</small>",
		                                             escaped,
		                                             secondary);

		g_free (secondary);
		g_free (escaped);
		g_free (primary);

		g_signal_connect (dialog,
		                  "response",
		                  G_CALLBACK (gtk_widget_destroy),
		                  NULL);

		gtk_widget_show (dialog);

#ifndef GEDIT_STABLE
	}
#endif

	/* This will also clean up the session */
	close_subscription (subscription);
}

static void
on_join_user_request_failed (InfcRequest  *request,
                             const GError *error,
                             Subscription *subscription)
{
	if (error->domain == inf_user_error_quark () &&
	    error->code == INF_USER_ERROR_NAME_IN_USE)
	{
		/* Generate a new name and try again */
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
		handle_error (subscription, error);
	}
}

static void
on_synchronization_failed (InfSession       *session,
                           InfXmlConnection *connection,
                           const GError     *error,
                           Subscription     *subscription)
{
	handle_error (subscription, error);
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
	GtkTextIter start;
	GtkTextIter end;

	GParameter parameters[] = {
		{"vector", {0,}},
		{"name", {0,}},
		{"caret-position", {0,}},
		{"selection-length", {0,}},
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

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gedit_tab_get_view (subscription->tab)));
	gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

	g_value_init (&parameters[2].value, G_TYPE_UINT);
	g_value_set_uint (&parameters[2].value, gtk_text_iter_get_offset (&start));

	g_value_init (&parameters[3].value, G_TYPE_INT);
	g_value_set_int (&parameters[3].value,
	                 gtk_text_iter_get_offset (&end) -
	                 gtk_text_iter_get_offset (&start));

	g_value_init (&parameters[4].value, G_TYPE_DOUBLE);
	g_value_set_double (&parameters[4].value, gedit_collaboration_user_get_hue (subscription->user));

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
		handle_error (subscription, error);
		g_error_free (error);
		return;
	}

	g_signal_connect_after (request,
	                        "failed",
	                        G_CALLBACK (on_join_user_request_failed),
	                        subscription);

	g_signal_connect_after (request,
	                        "finished",
	                        G_CALLBACK (on_join_user_request_finished),
	                        subscription);
}

#ifndef GEDIT_STABLE
static gchar *
guess_content_type (Subscription *subscription)
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *text;
	gchar *content_type;
	GeditDocument *doc;

	doc = gedit_tab_get_document (subscription->tab);

	gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (doc), &start);

	end = start;
	gtk_text_iter_forward_chars (&end, 100);

	text = gtk_text_iter_get_text (&start, &end);
	content_type = g_content_type_guess (gedit_document_get_short_name_for_display (doc),
	                                     (const guchar *)text,
	                                     strlen (text),
	                                     NULL);

	g_free (text);
	return content_type;
}
#endif

static void
on_synchronization_complete (InfSession       *session,
                             InfXmlConnection *connection,
                             Subscription     *subscription)
{
	gchar *content_type;

	g_signal_handler_disconnect (session,
	                             subscription->signal_handlers[SYNCHRONIZATION_COMPLETE]);
	subscription->signal_handlers[SYNCHRONIZATION_COMPLETE] = 0;

	g_signal_handler_disconnect (session,
	                             subscription->signal_handlers[SYNCHRONIZATION_PROGRESS]);
	subscription->signal_handlers[SYNCHRONIZATION_PROGRESS] = 0;

#ifndef GEDIT_STABLE
	gedit_tab_set_info_bar (subscription->tab, NULL);

	/* Now guess with the content too */
	content_type = guess_content_type (subscription);
	gedit_document_set_content_type (gedit_tab_get_document (subscription->tab),
	                                 content_type);
	g_free (content_type);
#endif

	subscription->progress_area = NULL;

	g_timer_destroy (subscription->progress_timer);
	subscription->progress_timer = NULL;

	request_join (subscription, NULL);
}

static void
on_progress_response (Subscription *subscription)
{
	gedit_window_close_tab (subscription->manager->priv->window,
	                        subscription->tab);
}

static void
on_synchronization_progress (InfSession       *session,
                             InfXmlConnection *connection,
                             gdouble           progress,
                             Subscription     *subscription)
{
#ifndef GEDIT_STABLE
	if (subscription->progress_area != NULL)
	{
		GeditCollaborationDocumentMessage *msg;
		gdouble fraction;

		fraction = (progress - subscription->progress_start) / (1 - subscription->progress_start);

		msg = GEDIT_COLLABORATION_DOCUMENT_MESSAGE (subscription->progress_area);
		gedit_collaboration_document_message_update (msg, fraction);
	}
	else if (g_timer_elapsed (subscription->progress_timer, NULL) > 0.5 && progress < 0.5)
	{
		subscription->progress_start = progress;
		subscription->progress_area =
			gedit_collaboration_document_message_new_progress (_("Synchronizing document"),
			                                                   _("Please wait while the shared document is being synchronized"));

		g_signal_connect_swapped (subscription->progress_area,
		                          "response",
		                          G_CALLBACK (on_progress_response),
		                          subscription);

		gtk_widget_show (subscription->progress_area);
		gedit_tab_set_info_bar (subscription->tab,
		                        subscription->progress_area);
	}
#endif
}

static void
on_subscribe_request_failed (InfcNodeRequest *request,
                             const GError    *error,
                             Subscription    *subscription)
{
	handle_error (subscription, error);
}

static void
on_session_close (InfSession   *session,
                  Subscription *subscription)
{
	GError *error;

	error = g_error_new (GEDIT_COLLABORATION_ERROR,
	                     GEDIT_COLLABORATION_ERROR_SESSION_CLOSED,
	                     "Collaboration session was closed");

	handle_error (subscription, error);
	g_error_free (error);
}

static void
on_connection_status (InfXmlConnection *connection,
                      GParamSpec       *spec,
                      Subscription     *subscription)
{
	InfXmlConnectionStatus status;

	g_object_get (connection, "status", &status, NULL);

	if (status == INF_XML_CONNECTION_CLOSED)
	{
		if (subscription->proxy)
		{
			inf_session_close (infc_session_proxy_get_session (subscription->proxy));
		}
	}
}

static void
on_subscribe_request_finished (InfcNodeRequest *request,
                               InfcBrowserIter *iter,
                               Subscription    *subscription)
{
	InfcSessionProxy *proxy;
	InfSession *session;
	GeditCollaborationManager *manager = subscription->manager;
	GeditView *view;
	GeditDocument *doc;
	gchar *content_type;
	const gchar *name;

	proxy = infc_browser_iter_get_session (subscription->browser, iter);
	session = infc_session_proxy_get_session (proxy);

	subscription->proxy = proxy;
	g_hash_table_insert (manager->priv->subscription_map,
	                     g_object_ref (proxy),
	                     subscription);

	subscription->tab = g_object_get_data (G_OBJECT (session), SESSION_TAB_DATA_KEY);
	g_object_set_data (G_OBJECT (subscription->tab), TAB_SUBSCRIPTION_DATA_KEY, subscription);

	view = gedit_tab_get_view (subscription->tab);
	doc = gedit_tab_get_document (subscription->tab);

	name = infc_browser_iter_get_name (subscription->browser, &subscription->iter);

#ifndef GEDIT_STABLE
	/* First guess the content type just from the name */
	content_type = g_content_type_guess (name, NULL, 0, NULL);
	gedit_document_set_content_type (doc, content_type);
	g_free (content_type);
#endif

	gtk_source_buffer_begin_not_undoable_action (GTK_SOURCE_BUFFER (doc));
	gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (doc));

	subscription->loading = TRUE;

#ifndef GEDIT_STABLE
	gedit_document_set_short_name_for_display (doc, name);
#endif

	subscription->signal_handlers[STYLE_SET] =
		g_signal_connect (view,
		                  "style-set",
		                  G_CALLBACK (on_style_set),
		                  inf_session_get_buffer (session));

	subscription->signal_handlers[VIEW_DESTROYED] =
		g_signal_connect (view,
		                 "destroy",
		                 G_CALLBACK (on_view_destroyed),
		                 subscription);

	subscription->signal_handlers[SYNCHRONIZATION_FAILED] =
		g_signal_connect_after (session,
		                        "synchronization-failed",
		                        G_CALLBACK (on_synchronization_failed),
		                        subscription);

	subscription->signal_handlers[SYNCHRONIZATION_COMPLETE] =
		g_signal_connect_after (session,
		                        "synchronization-complete",
		                        G_CALLBACK (on_synchronization_complete),
		                        subscription);

	subscription->signal_handlers[SYNCHRONIZATION_PROGRESS] =
		g_signal_connect_after (session,
		                        "synchronization-progress",
		                        G_CALLBACK (on_synchronization_progress),
		                        subscription);

	subscription->signal_handlers[SESSION_CLOSE] =
		g_signal_connect_after (session,
		                        "close",
		                        G_CALLBACK (on_session_close),
		                        subscription);

	subscription->signal_handlers[CONNECTION_STATUS] =
		g_signal_connect (infc_session_proxy_get_connection (subscription->proxy),
		                  "notify::status",
		                   G_CALLBACK (on_connection_status),
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
	InfcSessionProxy *proxy;

	g_return_val_if_fail (GEDIT_COLLABORATION_IS_MANAGER (manager), NULL);
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_USER (user), NULL);
	g_return_val_if_fail (INFC_IS_BROWSER (browser), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	proxy = infc_browser_iter_get_session (browser, iter);

	if (proxy != NULL)
	{
		/* Is already subscribed */
		subscription = g_hash_table_lookup (manager->priv->subscription_map,
		                                    proxy);

		if (subscription)
		{
			gedit_window_set_active_tab (manager->priv->window,
			                             subscription->tab);
		}

		return NULL;
	}

	if (infc_browser_iter_get_subscribe_request (browser, iter) != NULL)
	{
		/* In the middle of a request, just do nothing */
		return NULL;
	}

	connection = infc_browser_get_connection (browser);
	request = infc_browser_iter_subscribe_session (browser, iter);

	subscription = g_slice_new0 (Subscription);
	subscription->browser = g_object_ref (browser);
	subscription->iter = *iter;
	subscription->user = g_object_ref (user);
	subscription->manager = manager;
	subscription->progress_timer = g_timer_new ();

	manager->priv->subscriptions = g_slist_prepend (manager->priv->subscriptions,
	                                                subscription);

	g_signal_connect_after (request,
	                        "failed",
	                        G_CALLBACK (on_subscribe_request_failed),
	                        subscription);

	g_signal_connect_after (request,
	                        "finished",
	                        G_CALLBACK (on_subscribe_request_finished),
	                        subscription);

	return request;
}

static void
set_show_colors (GeditCollaborationManager *manager,
                 GeditTab                  *tab,
                 gboolean                   show_colors)
{
	Subscription *subscription;

	subscription = g_object_get_data (G_OBJECT (tab),
	                                  TAB_SUBSCRIPTION_DATA_KEY);

	if (subscription)
	{
		InfSession *session;
		InfTextGtkBuffer *buffer;
		GtkTextIter start;
		GtkTextIter end;

		session = infc_session_proxy_get_session (subscription->proxy);
		buffer = INF_TEXT_GTK_BUFFER (inf_session_get_buffer (session));

		gtk_text_buffer_get_bounds (inf_text_gtk_buffer_get_text_buffer (buffer),
		                            &start,
		                            &end);

		inf_text_gtk_buffer_show_user_colors (buffer,
		                                      show_colors,
		                                      &start,
		                                      &end);
	}
}

void
gedit_collaboration_manager_clear_colors (GeditCollaborationManager *manager,
                                          GeditTab                  *tab)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_MANAGER (manager));
	g_return_if_fail (GEDIT_IS_TAB (tab));

	set_show_colors (manager, tab, FALSE);
}

gboolean
gedit_collaboration_manager_tab_is_managed (GeditCollaborationManager *manager,
                                            GeditTab                  *tab)
{
	g_return_val_if_fail (GEDIT_COLLABORATION_IS_MANAGER (manager), FALSE);
	g_return_val_if_fail (GEDIT_IS_TAB (tab), FALSE);

	return g_object_get_data (G_OBJECT (tab),
	                          TAB_SUBSCRIPTION_DATA_KEY) != NULL;
}
