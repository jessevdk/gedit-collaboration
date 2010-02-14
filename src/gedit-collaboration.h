/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_H__
#define __GEDIT_COLLABORATION_H__

#include <gtk/gtk.h>

#define DEFAULT_INFINOTE_PORT 6523
#define DEFAULT_USER_HUE 0.6

G_BEGIN_DECLS

#define GEDIT_COLLABORATION_ERROR gedit_collaboration_error_quark ()

enum
{
	GEDIT_COLLABORATION_ERROR_SESSION_CLOSED
};

GQuark gedit_collaboration_error_quark (void);

void gedit_collaboration_get_sv (GtkWidget *widget,
                                 gdouble   *sat,
                                 gdouble   *val);

void gedit_collaboration_hue_to_color (gdouble hue, GdkColor *color);
gdouble gedit_collaboration_color_to_hue (GdkColor *color);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_H__ */

