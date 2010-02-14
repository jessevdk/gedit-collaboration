/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_PLUGIN_H__
#define __GEDIT_COLLABORATION_PLUGIN_H__

#include <gedit/gedit-plugin.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_COLLABORATION_PLUGIN				(gedit_collaboration_plugin_get_type ())
#define GEDIT_COLLABORATION_PLUGIN(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPlugin))
#define GEDIT_COLLABORATION_PLUGIN_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPlugin const))
#define GEDIT_COLLABORATION_PLUGIN_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPluginClass))
#define GEDIT_IS_COLLABORATION_PLUGIN(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_COLLABORATION_PLUGIN))
#define GEDIT_IS_COLLABORATION_PLUGIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_COLLABORATION_PLUGIN))
#define GEDIT_COLLABORATION_PLUGIN_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPluginClass))

typedef struct _GeditCollaborationPlugin		GeditCollaborationPlugin;
typedef struct _GeditCollaborationPluginClass	GeditCollaborationPluginClass;
typedef struct _GeditCollaborationPluginPrivate	GeditCollaborationPluginPrivate;

struct _GeditCollaborationPlugin {
	GeditPlugin parent;

	GeditCollaborationPluginPrivate *priv;
};

struct _GeditCollaborationPluginClass {
	GeditPluginClass parent_class;
};

GType gedit_collaboration_plugin_get_type (void) G_GNUC_CONST;

G_MODULE_EXPORT GType gedit_register_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_PLUGIN_H__ */
