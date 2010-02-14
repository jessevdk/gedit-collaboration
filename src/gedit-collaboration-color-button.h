/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_COLOR_BUTTON_H__
#define __GEDIT_COLLABORATION_COLOR_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_TYPE_COLOR_BUTTON				(gedit_collaboration_color_button_get_type ())
#define GEDIT_COLLABORATION_COLOR_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON, GeditCollaborationColorButton))
#define GEDIT_COLLABORATION_COLOR_BUTTON_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON, GeditCollaborationColorButton const))
#define GEDIT_COLLABORATION_COLOR_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON, GeditCollaborationColorButtonClass))
#define GEDIT_COLLABORATION_IS_COLOR_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON))
#define GEDIT_COLLABORATION_IS_COLOR_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON))
#define GEDIT_COLLABORATION_COLOR_BUTTON_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON, GeditCollaborationColorButtonClass))

typedef struct _GeditCollaborationColorButton			GeditCollaborationColorButton;
typedef struct _GeditCollaborationColorButtonClass		GeditCollaborationColorButtonClass;
typedef struct _GeditCollaborationColorButtonPrivate	GeditCollaborationColorButtonPrivate;

struct _GeditCollaborationColorButton {
	GtkColorButton parent;
	
	GeditCollaborationColorButtonPrivate *priv;
};

struct _GeditCollaborationColorButtonClass {
	GtkColorButtonClass parent_class;
};

GType gedit_collaboration_color_button_get_type (void) G_GNUC_CONST;
GType gedit_collaboration_color_button_register_type (GTypeModule *type_module);

GeditCollaborationColorButton *gedit_collaboration_color_button_new (void);

void gedit_collaboration_color_button_set_hue (GeditCollaborationColorButton *button,
                                               gdouble                        hue);

gdouble gedit_collaboration_color_button_get_hue (GeditCollaborationColorButton *button);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_COLOR_BUTTON_H__ */
