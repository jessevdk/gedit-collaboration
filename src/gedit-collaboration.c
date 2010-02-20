/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration.h"
#include <math.h>

GQuark
gedit_collaboration_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_string ("gedit_collaboration_error");
	}

	return quark;
}

void
gedit_collaboration_get_sv (GtkWidget *widget,
                            gdouble   *sat,
                            gdouble   *val)
{
	GdkColor color;
	GtkStyle *style;
	gdouble r, g, b;
	gdouble h;

	style = gtk_widget_get_style (widget);
	color = style->base[gtk_widget_get_state (widget)];

	r = color.red / 65535.0;
	g = color.green / 65535.0;
	b = color.blue / 65535.0;

	gtk_rgb_to_hsv (r, g, b, &h, sat, val);

	*sat = *sat * 0.5 + 0.3;
	*val = (pow(*val + 1, 3) - 1) / 7 * 0.6 + 0.4;
}

void
gedit_collaboration_hue_to_color (gdouble   hue,
                                  GdkColor *color)
{
	gdouble r, g, b;

	gtk_hsv_to_rgb (hue, 0.5, 0.5, &r, &g, &b);

	color->red = r * 65535;
	color->green = g * 65535;
	color->blue = b * 65535;
}

gdouble
gedit_collaboration_color_to_hue (GdkColor *color)
{
	gdouble r, g, b;
	gdouble h, s, v;

	r = color->red / 65535.0;
	g = color->green / 65535.0;
	b = color->blue / 65535.0;

	gtk_rgb_to_hsv (r, g, b, &h, &s, &v);
	return h;
}

static GtkBuilder *
try_create_builder (const gchar  *data_dir,
                    const gchar  *filename,
                    GError      **error)
{
	GtkBuilder *builder;
	gchar *path;

	builder = gtk_builder_new ();

	path = g_build_filename (data_dir, filename, NULL);

	if (!gtk_builder_add_from_file (builder, path, error))
	{
		g_object_unref (builder);
		builder = NULL;
	}

	g_free (path);
	return builder;
}

GtkBuilder *
gedit_collaboration_create_builder (const gchar *data_dir,
                                    const gchar *filename)
{
	GtkBuilder *builder;
	GError *error = NULL;

	builder = try_create_builder (data_dir, filename, NULL);

	if (builder == NULL)
	{
		builder = try_create_builder (GEDIT_PLUGINS_DATA_DIR "/collaboration",
		                              filename,
		                              &error);
	}

	if (builder == NULL)
	{
		g_warning ("Could not construct builder for file %s", filename);
		g_error_free (error);
	}

	return builder;
}
