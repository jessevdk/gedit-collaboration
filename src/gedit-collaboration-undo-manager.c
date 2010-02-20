/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-undo-manager.h"

#include <gedit/gedit-plugin.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>
#include <libinfinity/adopted/inf-adopted-undo-grouping.h>
#include <libinftext/inf-text-undo-grouping.h>

#define GEDIT_COLLABORATION_UNDO_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER, GeditCollaborationUndoManagerPrivate))

enum
{
	BEGIN_USER_ACTION,
	END_USER_ACTION,
	NUM_SIGNALS_TEXT_BUFFER,
	CAN_UNDO_CHANGED,
	CAN_REDO_CHANGED,
	NUM_SIGNALS_ALGORITHM,
	NUM_SIGNALS
};

struct _GeditCollaborationUndoManagerPrivate
{
	InfAdoptedSession *session;
	InfAdoptedUser *user;
	InfAdoptedUndoGrouping *grouping;

	guint signals[NUM_SIGNALS];
};

/* Properties */
enum
{
	PROP_0,
	PROP_SESSION,
	PROP_USER
};

static void gedit_collaboration_undo_manager_iface_init (GtkSourceUndoManagerIface *iface);

GEDIT_PLUGIN_DEFINE_TYPE_WITH_CODE (GeditCollaborationUndoManager, gedit_collaboration_undo_manager, G_TYPE_OBJECT,
                                    GEDIT_PLUGIN_IMPLEMENT_INTERFACE (gedit_collaboration_undo_manager,
                                                                      GTK_TYPE_SOURCE_UNDO_MANAGER,
                                                                      gedit_collaboration_undo_manager_iface_init))

static void
gedit_collaboration_undo_manager_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_undo_manager_parent_class)->finalize (object);
}

static void
uninstall_buffer_handlers (GeditCollaborationUndoManager *manager)
{
	InfTextGtkBuffer *buffer;
	GtkTextBuffer *text_buffer;
	gint i;

	buffer = INF_TEXT_GTK_BUFFER (inf_session_get_buffer (INF_SESSION (manager->priv->session)));
	text_buffer = inf_text_gtk_buffer_get_text_buffer (buffer);

	for (i = 0; i < NUM_SIGNALS_TEXT_BUFFER; ++i)
	{
		g_signal_handler_disconnect (text_buffer,
		                             manager->priv->signals[i]);
	}
}

static void
on_can_undo_changed (InfAdoptedAlgorithm           *algorithm,
                     InfAdoptedUser                *user,
                     gboolean                       can_undo,
                     GeditCollaborationUndoManager *manager)
{
	if (user == manager->priv->user)
	{
		gtk_source_undo_manager_can_undo_changed (GTK_SOURCE_UNDO_MANAGER (manager));
	}
}

static void
on_can_redo_changed (InfAdoptedAlgorithm           *algorithm,
                     InfAdoptedUser                *user,
                     gboolean                       can_redo,
                     GeditCollaborationUndoManager *manager)
{
	if (user == manager->priv->user)
	{
		gtk_source_undo_manager_can_redo_changed (GTK_SOURCE_UNDO_MANAGER (manager));
	}
}

static void
on_begin_user_action (GtkTextBuffer                 *buffer,
                      GeditCollaborationUndoManager *manager)
{
	/*inf_adopted_undo_grouping_start_group (manager->priv->grouping,
	                                       TRUE);*/
}

static void
on_end_user_action (GtkTextBuffer                 *buffer,
                    GeditCollaborationUndoManager *manager)
{
	/*inf_adopted_undo_grouping_end_group (manager->priv->grouping,
	                                     TRUE);*/
}

static void
install_buffer_handlers (GeditCollaborationUndoManager *manager)
{
	InfTextGtkBuffer *buffer;
	GtkTextBuffer *text_buffer;

	buffer = INF_TEXT_GTK_BUFFER (inf_session_get_buffer (INF_SESSION (manager->priv->session)));
	text_buffer = inf_text_gtk_buffer_get_text_buffer (buffer);

	manager->priv->signals[BEGIN_USER_ACTION] =
		g_signal_connect (text_buffer,
		                  "begin-user-action",
		                  G_CALLBACK (on_begin_user_action),
		                  manager);

	manager->priv->signals[END_USER_ACTION] =
		g_signal_connect (text_buffer,
		                  "end-user-action",
		                  G_CALLBACK (on_end_user_action),
		                  manager);
}

static void
set_session (GeditCollaborationUndoManager *manager,
             InfAdoptedSession             *session)
{
	if (session == manager->priv->session)
	{
		return;
	}

	if (manager->priv->session)
	{
		gint i;
		InfAdoptedAlgorithm *algorithm;

		algorithm = inf_adopted_session_get_algorithm (manager->priv->session);

		for (i = NUM_SIGNALS_TEXT_BUFFER + 1; i < NUM_SIGNALS_ALGORITHM; ++i)
		{
			g_signal_handler_disconnect (algorithm,
			                             manager->priv->signals[i]);
		}

		uninstall_buffer_handlers (manager);

		g_object_unref (manager->priv->grouping);
		manager->priv->grouping = NULL;

		g_object_unref (manager->priv->session);

		manager->priv->session = NULL;
	}

	if (session)
	{
		InfAdoptedAlgorithm *algorithm;

		manager->priv->session = g_object_ref (session);
		algorithm = inf_adopted_session_get_algorithm (manager->priv->session);

		manager->priv->signals[CAN_UNDO_CHANGED] =
			g_signal_connect_after (algorithm,
			                        "can-undo-changed",
			                        G_CALLBACK (on_can_undo_changed),
			                        manager);

		manager->priv->signals[CAN_REDO_CHANGED] =
			g_signal_connect_after (algorithm,
			                        "can-redo-changed",
			                        G_CALLBACK (on_can_redo_changed),
			                        manager);

		install_buffer_handlers (manager);

		manager->priv->grouping = INF_ADOPTED_UNDO_GROUPING (inf_text_undo_grouping_new ());

		inf_adopted_undo_grouping_set_algorithm (manager->priv->grouping,
		                                         inf_adopted_session_get_algorithm (manager->priv->session),
		                                         manager->priv->user);
	}
}

static void
gedit_collaboration_undo_manager_dispose (GObject *object)
{
	GeditCollaborationUndoManager *manager = GEDIT_COLLABORATION_UNDO_MANAGER (object);

	if (manager->priv->session)
	{
		set_session (manager, NULL);
	}

	if (manager->priv->user)
	{
		g_object_unref (manager->priv->user);
		manager->priv->user = NULL;
	}

	G_OBJECT_CLASS (gedit_collaboration_undo_manager_parent_class)->dispose (object);
}

static void
gedit_collaboration_undo_manager_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec)
{
	GeditCollaborationUndoManager *self = GEDIT_COLLABORATION_UNDO_MANAGER (object);
	
	switch (prop_id)
	{
		case PROP_SESSION:
			set_session (self, g_value_get_object (value));
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
gedit_collaboration_undo_manager_get_property (GObject    *object,
                                               guint       prop_id,
                                               GValue     *value,
                                               GParamSpec *pspec)
{
	GeditCollaborationUndoManager *self = GEDIT_COLLABORATION_UNDO_MANAGER (object);
	
	switch (prop_id)
	{
		case PROP_SESSION:
			g_value_set_object (value, self->priv->session);
		break;
		case PROP_USER:
			g_value_set_object (value, self->priv->user);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static GObject *
gedit_collaboration_undo_manager_constructor (GType                  type,
                                              guint                  n_construct_properties,
                                              GObjectConstructParam *construct_properties)
{
	GeditCollaborationUndoManager *manager;
	GObject *object;

	object = G_OBJECT_CLASS (gedit_collaboration_undo_manager_parent_class)->constructor (
		type,
		n_construct_properties,
		construct_properties
	);

	manager = GEDIT_COLLABORATION_UNDO_MANAGER (object);

	if (manager->priv->session == NULL)
	{
		g_warning ("Need session to construct undo manager");
		g_object_unref (object);

		return NULL;
	}

	if (manager->priv->user == NULL)
	{
		g_warning ("Need user to construct undo manager");
		g_object_unref (object);

		return NULL;
	}

	return object;
}

static void
gedit_collaboration_undo_manager_class_init (GeditCollaborationUndoManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_undo_manager_finalize;
	object_class->dispose = gedit_collaboration_undo_manager_dispose;
	object_class->constructor = gedit_collaboration_undo_manager_constructor;

	object_class->set_property = gedit_collaboration_undo_manager_set_property;
	object_class->get_property = gedit_collaboration_undo_manager_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_SESSION,
	                                 g_param_spec_object ("session",
	                                                      "Session",
	                                                      "Session",
	                                                      INF_ADOPTED_TYPE_SESSION,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_USER,
	                                 g_param_spec_object ("user",
	                                                      "User",
	                                                      "User",
	                                                      INF_ADOPTED_TYPE_USER,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationUndoManagerPrivate));
}

static void
gedit_collaboration_undo_manager_init (GeditCollaborationUndoManager *self)
{
	self->priv = GEDIT_COLLABORATION_UNDO_MANAGER_GET_PRIVATE (self);
}

GeditCollaborationUndoManager *
gedit_collaboration_undo_manager_new (InfAdoptedSession *session,
                                      InfAdoptedUser    *user)
{
	return g_object_new (GEDIT_COLLABORATION_TYPE_UNDO_MANAGER,
	                     "user", user,
	                     "session", session,
	                     NULL);
}

static gboolean
undo_manager_can_undo_impl (GtkSourceUndoManager *manager)
{
	GeditCollaborationUndoManager *undo_manager;
	InfAdoptedAlgorithm *algorithm;

	undo_manager = GEDIT_COLLABORATION_UNDO_MANAGER (manager);

	algorithm = inf_adopted_session_get_algorithm (undo_manager->priv->session);
	return inf_adopted_algorithm_can_undo (algorithm, undo_manager->priv->user);
}

static gboolean
undo_manager_can_redo_impl (GtkSourceUndoManager *manager)
{
	GeditCollaborationUndoManager *undo_manager;
	InfAdoptedAlgorithm *algorithm;

	undo_manager = GEDIT_COLLABORATION_UNDO_MANAGER (manager);

	algorithm = inf_adopted_session_get_algorithm (undo_manager->priv->session);
	return inf_adopted_algorithm_can_redo (algorithm, undo_manager->priv->user);
}

static void
undo_manager_undo_impl (GtkSourceUndoManager *manager)
{
	GeditCollaborationUndoManager *undo_manager;

	undo_manager = GEDIT_COLLABORATION_UNDO_MANAGER (manager);

	inf_adopted_session_undo (undo_manager->priv->session,
	                          undo_manager->priv->user,
	                          inf_adopted_undo_grouping_get_undo_size (undo_manager->priv->grouping));
}

static void
undo_manager_redo_impl (GtkSourceUndoManager *manager)
{
	GeditCollaborationUndoManager *undo_manager;

	undo_manager = GEDIT_COLLABORATION_UNDO_MANAGER (manager);

	inf_adopted_session_redo (undo_manager->priv->session,
	                          undo_manager->priv->user,
	                          inf_adopted_undo_grouping_get_redo_size (undo_manager->priv->grouping));
}

static void
gedit_collaboration_undo_manager_iface_init (GtkSourceUndoManagerIface *iface)
{
	iface->can_undo = undo_manager_can_undo_impl;
	iface->can_redo = undo_manager_can_redo_impl;

	iface->undo = undo_manager_undo_impl;
	iface->redo = undo_manager_redo_impl;
}
