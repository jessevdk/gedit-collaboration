/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_UNDO_MANAGER_H__
#define __GEDIT_COLLABORATION_UNDO_MANAGER_H__

#include <glib-object.h>
#include <libinfinity/adopted/inf-adopted-session.h>

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_UNDO_MANAGER			(gedit_collaboration_undo_manager_get_type ())
#define GEDIT_COLLABORATION_UNDO_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER, GeditCollaborationUndoManager))
#define GEDIT_COLLABORATION_UNDO_MANAGER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER, GeditCollaborationUndoManager const))
#define GEDIT_COLLABORATION_UNDO_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER, GeditCollaborationUndoManagerClass))
#define GEDIT_COLLABORATION_IS_UNDO_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER))
#define GEDIT_COLLABORATION_IS_UNDO_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER))
#define GEDIT_COLLABORATION_UNDO_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_UNDO_MANAGER, GeditCollaborationUndoManagerClass))

typedef struct _GeditCollaborationUndoManager		GeditCollaborationUndoManager;
typedef struct _GeditCollaborationUndoManagerClass	GeditCollaborationUndoManagerClass;
typedef struct _GeditCollaborationUndoManagerPrivate	GeditCollaborationUndoManagerPrivate;

struct _GeditCollaborationUndoManager
{
	GObject parent;

	GeditCollaborationUndoManagerPrivate *priv;
};

struct _GeditCollaborationUndoManagerClass
{
	GObjectClass parent_class;
};

GType gedit_collaboration_undo_manager_get_type (void) G_GNUC_CONST;
void _gedit_collaboration_undo_manager_register_type (GTypeModule *type_module);

GeditCollaborationUndoManager *gedit_collaboration_undo_manager_new (InfAdoptedSession *session,
                                                                     InfAdoptedUser    *user);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_UNDO_MANAGER_H__ */
