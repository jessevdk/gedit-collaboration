/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_MANAGER_H__
#define __GEDIT_COLLABORATION_MANAGER_H__

#include <glib-object.h>
#include <gedit/gedit-window.h>
#include <libinfinity/client/infc-note-plugin.h>
#include <libinfinity/client/infc-browser.h>
#include "gedit-collaboration-user.h"

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_MANAGER			(gedit_collaboration_manager_get_type ())
#define GEDIT_COLLABORATION_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_MANAGER, GeditCollaborationManager))
#define GEDIT_COLLABORATION_MANAGER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_MANAGER, GeditCollaborationManager const))
#define GEDIT_COLLABORATION_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_MANAGER, GeditCollaborationManagerClass))
#define GEDIT_COLLABORATION_IS_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_MANAGER))
#define GEDIT_COLLABORATION_IS_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_MANAGER))
#define GEDIT_COLLABORATION_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_MANAGER, GeditCollaborationManagerClass))

typedef struct _GeditCollaborationManager			GeditCollaborationManager;
typedef struct _GeditCollaborationManagerClass		GeditCollaborationManagerClass;
typedef struct _GeditCollaborationManagerPrivate	GeditCollaborationManagerPrivate;

struct _GeditCollaborationManager {
	GObject parent;

	GeditCollaborationManagerPrivate *priv;
};

struct _GeditCollaborationManagerClass {
	GObjectClass parent_class;
};

GType gedit_collaboration_manager_get_type (void) G_GNUC_CONST;
GType gedit_collaboration_manager_register_type (GTypeModule *type_module);

GeditCollaborationManager *gedit_collaboration_manager_new (GeditWindow *window);

InfcNotePlugin *gedit_collaboration_manager_get_note_plugin (GeditCollaborationManager *manager);
InfcNodeRequest *gedit_collaboration_manager_subscribe (GeditCollaborationManager *manager,
                                                        GeditCollaborationUser    *user,
                                                        InfcBrowser               *browser,
                                                        const InfcBrowserIter     *iter);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_MANAGER_H__ */
