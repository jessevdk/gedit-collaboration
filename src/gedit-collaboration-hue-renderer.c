/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-hue-renderer.h"
#include <gedit/gedit-plugin.h>
#include "gedit-collaboration.h"

#define GEDIT_COLLABORATION_HUE_RENDERER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_COLLABORATION_HUE_RENDERER, GeditCollaborationHueRendererPrivate))

struct _GeditCollaborationHueRendererPrivate
{
	gdouble hue;
};

enum
{
	PROP_0,
	PROP_HUE
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationHueRenderer, gedit_collaboration_hue_renderer, GTK_TYPE_CELL_RENDERER)

static void
gedit_collaboration_hue_renderer_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_hue_renderer_parent_class)->finalize (object);
}

static void
gedit_collaboration_hue_renderer_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec)
{
	GeditCollaborationHueRenderer *self = GEDIT_COLLABORATION_HUE_RENDERER (object);
	
	switch (prop_id)
	{
		case PROP_HUE:
			self->priv->hue = g_value_get_double (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_hue_renderer_get_property (GObject    *object,
                                               guint       prop_id,
                                               GValue     *value,
                                               GParamSpec *pspec)
{
	GeditCollaborationHueRenderer *self = GEDIT_COLLABORATION_HUE_RENDERER (object);
	
	switch (prop_id)
	{
		case PROP_HUE:
			g_value_set_double (value, self->priv->hue);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_hue_renderer_get_size (GtkCellRenderer *cell,
                                           GtkWidget       *widget,
                                           GdkRectangle    *cell_area,
                                           gint            *x_offset,
                                           gint            *y_offset,
                                           gint            *width,
                                           gint            *height)
{
	gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (widget),
	                                   GTK_ICON_SIZE_MENU,
	                                   width,
	                                   height);

	if (x_offset)
	{
		*x_offset = 0;
	}

	if (y_offset)
	{
		*y_offset = 0;
	}
}

static void
set_source_color_from_hue (GeditCollaborationHueRenderer *renderer,
                           GtkWidget                     *widget,
                           cairo_t                       *ctx,
                           gdouble                        y,
                           gdouble                        height)
{
	gdouble s, v;
	gdouble r, g, b;
	cairo_pattern_t *pattern;

	gedit_collaboration_get_sv (widget, &s, &v);

	pattern = cairo_pattern_create_linear (0, y, 0, y + height);

	gtk_hsv_to_rgb (renderer->priv->hue, s, v, &r, &g, &b);
	cairo_pattern_add_color_stop_rgb (pattern, 0, r, g, b);

	v *= v < 0.5 ? 1.2 : 0.8;

	gtk_hsv_to_rgb (renderer->priv->hue, s, v, &r, &g, &b);
	cairo_pattern_add_color_stop_rgb (pattern, 1, r, g, b);

	cairo_set_source (ctx, pattern);
	cairo_pattern_destroy (pattern);
}

static void
gedit_collaboration_hue_renderer_render (GtkCellRenderer      *cell,
                                         GdkDrawable          *window,
                                         GtkWidget            *widget,
                                         GdkRectangle         *background_area,
                                         GdkRectangle         *cell_area,
                                         GdkRectangle         *expose_area,
                                         GtkCellRendererState  flags)
{
	gdouble xpad;
	gdouble ypad;
	GtkStyle *style;
	gdouble x;
	gdouble y;
	gdouble height;
	gdouble width;

	g_object_get (cell, "xpad", &xpad, "ypad", &ypad, NULL);

	style = gtk_widget_get_style (widget);

	/* Draw a nice little rectangle with the current hue in the
	   cell_area */
	cairo_t *ctx = gdk_cairo_create (window);

	gdk_cairo_rectangle (ctx, expose_area);
	cairo_clip (ctx);

	x = cell_area->x + xpad + 0.5;
	y = cell_area->y + ypad + 0.5;

	width = cell_area->width - 2 * xpad - 1;
	height = cell_area->height - 2 * ypad - 1;

	cairo_rectangle (ctx, x, y, width, height);

	set_source_color_from_hue (GEDIT_COLLABORATION_HUE_RENDERER (cell),
	                           widget,
	                           ctx,
	                           y,
	                           height);

	cairo_set_line_width (ctx, 1);

	cairo_fill_preserve (ctx);

	gdk_cairo_set_source_color (ctx, &style->fg[gtk_widget_get_state (widget)]);
	cairo_stroke (ctx);

	cairo_destroy (ctx);
}

static void
gedit_collaboration_hue_renderer_class_init (GeditCollaborationHueRendererClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkCellRendererClass *renderer_class = GTK_CELL_RENDERER_CLASS (klass);

	object_class->finalize = gedit_collaboration_hue_renderer_finalize;

	object_class->set_property = gedit_collaboration_hue_renderer_set_property;
	object_class->get_property = gedit_collaboration_hue_renderer_get_property;

	renderer_class->get_size = gedit_collaboration_hue_renderer_get_size;
	renderer_class->render = gedit_collaboration_hue_renderer_render;

	g_object_class_install_property (object_class,
	                                 PROP_HUE,
	                                 g_param_spec_double ("hue",
	                                                      "Hue",
	                                                      "Hue",
	                                                      0,
	                                                      1,
	                                                      0.0,
	                                                      G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationHueRendererPrivate));
}

static void
gedit_collaboration_hue_renderer_init (GeditCollaborationHueRenderer *self)
{
	self->priv = GEDIT_COLLABORATION_HUE_RENDERER_GET_PRIVATE (self);
}

GtkCellRenderer *
gedit_collaboration_hue_renderer_new ()
{
	return g_object_new (GEDIT_TYPE_COLLABORATION_HUE_RENDERER, NULL);
}
