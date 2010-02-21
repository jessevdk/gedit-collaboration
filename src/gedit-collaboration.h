/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_H__
#define __GEDIT_COLLABORATION_H__

#include <gtk/gtk.h>

#define DEFAULT_INFINOTE_PORT 6523

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

GtkBuilder *gedit_collaboration_create_builder (const gchar *data_dir,
                                                const gchar *filename);

gchar *gedit_collaboration_generate_new_name (const gchar *name,
                                              gint        *name_failed_counter);

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_H__ */

