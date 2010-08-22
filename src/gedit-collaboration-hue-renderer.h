/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_HUE_RENDERER_H__
#define __GEDIT_COLLABORATION_HUE_RENDERER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_COLLABORATION_HUE_RENDERER			(gedit_collaboration_hue_renderer_get_type ())
#define GEDIT_COLLABORATION_HUE_RENDERER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_COLLABORATION_HUE_RENDERER, GeditCollaborationHueRenderer))
#define GEDIT_COLLABORATION_HUE_RENDERER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_COLLABORATION_HUE_RENDERER, GeditCollaborationHueRenderer const))
#define GEDIT_COLLABORATION_HUE_RENDERER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_COLLABORATION_HUE_RENDERER, GeditCollaborationHueRendererClass))
#define GEDIT_IS_COLLABORATION_HUE_RENDERER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_COLLABORATION_HUE_RENDERER))
#define GEDIT_IS_COLLABORATION_HUE_RENDERER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_COLLABORATION_HUE_RENDERER))
#define GEDIT_COLLABORATION_HUE_RENDERER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_COLLABORATION_HUE_RENDERER, GeditCollaborationHueRendererClass))

typedef struct _GeditCollaborationHueRenderer		GeditCollaborationHueRenderer;
typedef struct _GeditCollaborationHueRendererClass	GeditCollaborationHueRendererClass;
typedef struct _GeditCollaborationHueRendererPrivate	GeditCollaborationHueRendererPrivate;

struct _GeditCollaborationHueRenderer
{
	GtkCellRenderer parent;

	GeditCollaborationHueRendererPrivate *priv;
};

struct _GeditCollaborationHueRendererClass
{
	GtkCellRendererClass parent_class;
};

GType gedit_collaboration_hue_renderer_get_type (void) G_GNUC_CONST;
void _gedit_collaboration_hue_renderer_register_type (GTypeModule *type_module);

GtkCellRenderer *gedit_collaboration_hue_renderer_new (void);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_HUE_RENDERER_H__ */
