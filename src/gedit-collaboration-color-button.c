/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-color-button.h"

#include <config.h>
#include <gedit/gedit-plugin.h>
#include <libinftextgtk/inf-text-gtk-hue-chooser.h>
#include <glib/gi18n-lib.h>
#include "gedit-collaboration.h"

#define GEDIT_COLLABORATION_COLOR_BUTTON_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_COLOR_BUTTON, GeditCollaborationColorButtonPrivate))

struct _GeditCollaborationColorButtonPrivate
{
	GtkWidget *color_dialog;
	GtkWidget *hue_chooser;
	gboolean modal;
};

/* Properties */
enum
{
	PROP_0,
	PROP_MODAL
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationColorButton, gedit_collaboration_color_button, GTK_TYPE_COLOR_BUTTON)

static void
gedit_collaboration_color_button_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_color_button_parent_class)->finalize (object);
}

static void
on_color_dialog_response (GtkWidget                     *dlg,
                          int                            responseid,
                          GeditCollaborationColorButton *self)
{
	if (responseid == GTK_RESPONSE_OK)
	{
		InfTextGtkHueChooser *chooser;
		gdouble hue;

		chooser = INF_TEXT_GTK_HUE_CHOOSER (self->priv->hue_chooser);
		hue = inf_text_gtk_hue_chooser_get_hue (chooser);

		gedit_collaboration_color_button_set_hue (self, hue);
	}

	self->priv->color_dialog = NULL;
	self->priv->hue_chooser = NULL;

	gtk_widget_destroy (dlg);
}

static void
color_button_clicked (GtkButton *button)
{
	GeditCollaborationColorButton *self = GEDIT_COLLABORATION_COLOR_BUTTON (button);

	if (self->priv->color_dialog == NULL)
	{
		GtkWidget *content;

		self->priv->color_dialog =
			gtk_dialog_new_with_buttons (_("Select User Color"),
			                             GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (button))),
			                             GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
			                             GTK_STOCK_CANCEL,
			                             GTK_RESPONSE_CANCEL,
			                             GTK_STOCK_OK,
			                             GTK_RESPONSE_OK,
			                             NULL);

		gtk_window_set_modal (GTK_WINDOW (self->priv->color_dialog),
		                      self->priv->modal);

		content = gtk_dialog_get_content_area (GTK_DIALOG (self->priv->color_dialog));
		self->priv->hue_chooser = inf_text_gtk_hue_chooser_new ();
		gtk_widget_show (self->priv->hue_chooser);

		gtk_box_pack_start (GTK_BOX (content),
		                    self->priv->hue_chooser,
		                    FALSE,
		                    FALSE,
		                    0);

		g_signal_connect (self->priv->color_dialog,
		                  "response",
		                  G_CALLBACK (on_color_dialog_response),
		                  self);
	}

	inf_text_gtk_hue_chooser_set_hue (INF_TEXT_GTK_HUE_CHOOSER (self->priv->hue_chooser),
	                                  gedit_collaboration_color_button_get_hue (self));

	gtk_widget_show (self->priv->color_dialog);
}

static void
gedit_collaboration_color_button_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GeditCollaborationColorButton *self = GEDIT_COLLABORATION_COLOR_BUTTON (object);

	switch (prop_id)
	{
		case PROP_MODAL:
			self->priv->modal = g_value_get_boolean (value);

			if (self->priv->color_dialog)
			{
				gtk_window_set_modal (GTK_WINDOW (self->priv->color_dialog),
				                      self->priv->modal);
			}
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_color_button_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GeditCollaborationColorButton *self = GEDIT_COLLABORATION_COLOR_BUTTON (object);

	switch (prop_id)
	{
		case PROP_MODAL:
			g_value_set_boolean (value, self->priv->modal);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_color_button_class_init (GeditCollaborationColorButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkButtonClass *button_class = GTK_BUTTON_CLASS (klass);

	object_class->finalize = gedit_collaboration_color_button_finalize;
	button_class->clicked = color_button_clicked;

	object_class->set_property = gedit_collaboration_color_button_set_property;
	object_class->get_property = gedit_collaboration_color_button_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_MODAL,
	                                 g_param_spec_boolean ("modal",
	                                                       "Modal",
	                                                       "Modal",
	                                                       FALSE,
	                                                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationColorButtonPrivate));
}

static void
gedit_collaboration_color_button_init (GeditCollaborationColorButton *self)
{
	self->priv = GEDIT_COLLABORATION_COLOR_BUTTON_GET_PRIVATE (self);
}

GeditCollaborationColorButton *
gedit_collaboration_color_button_new ()
{
	return g_object_new (GEDIT_COLLABORATION_TYPE_COLOR_BUTTON, NULL);
}

void
gedit_collaboration_color_button_set_hue (GeditCollaborationColorButton *button,
                                          gdouble                        hue)
{
	GdkColor color;

	g_return_if_fail (GEDIT_COLLABORATION_IS_COLOR_BUTTON (button));

	gedit_collaboration_hue_to_color (hue, &color);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (button), &color);
}

gdouble
gedit_collaboration_color_button_get_hue (GeditCollaborationColorButton *button)
{
	GdkColor color;

	g_return_val_if_fail (GEDIT_COLLABORATION_IS_COLOR_BUTTON (button), 0.0);

	gtk_color_button_get_color (GTK_COLOR_BUTTON (button),
	                            &color);

	return gedit_collaboration_color_to_hue (&color);
}
