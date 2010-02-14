/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-document-message.h"
#include <gedit/gedit-plugin.h>
#include "gedit-collaboration.h"
#include <config.h>
#include <glib/gi18n-lib.h>
#include <libinfinity/common/inf-error.h>
#include <libinfinity/adopted/inf-adopted-state-vector.h>
#include <libinfinity/adopted/inf-adopted-session.h>

#define GEDIT_COLLABORATION_DOCUMENT_MESSAGE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, GeditCollaborationDocumentMessagePrivate))

struct _GeditCollaborationDocumentMessagePrivate
{
	GtkWidget *progress;
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationDocumentMessage, gedit_collaboration_document_message, GTK_TYPE_INFO_BAR)

static void
gedit_collaboration_document_message_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_document_message_parent_class)->finalize (object);
}

static void
set_message_area_text_and_icon (GeditCollaborationDocumentMessage *message_area,
                                const gchar                       *icon_stock_id,
                                const gchar                       *primary_text,
                                const gchar                       *secondary_text,
                                gboolean                           progress)
{
	GtkWidget *hbox_content;
	GtkWidget *image;
	GtkWidget *vbox;
	gchar *primary_markup;
	gchar *secondary_markup;
	GtkWidget *primary_label;
	GtkWidget *secondary_label;
	GtkWidget *content_area;
	gchar *escaped;

	hbox_content = gtk_hbox_new (FALSE, 8);

	image = gtk_image_new_from_stock (icon_stock_id, GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start (GTK_BOX (hbox_content), image, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox_content), vbox, TRUE, TRUE, 0);

	escaped = g_markup_escape_text (primary_text, -1);
	primary_markup = g_strdup_printf ("<b>%s</b>", escaped);
	g_free (escaped);

	primary_label = gtk_label_new (primary_markup);
	g_free (primary_markup);
	gtk_box_pack_start (GTK_BOX (vbox), primary_label, TRUE, TRUE, 0);
	gtk_label_set_use_markup (GTK_LABEL (primary_label), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (primary_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (primary_label), 0, 0.5);
	GTK_WIDGET_SET_FLAGS (primary_label, GTK_CAN_FOCUS);
	gtk_label_set_selectable (GTK_LABEL (primary_label), TRUE);

	if (secondary_text != NULL)
	{
		escaped = g_markup_escape_text (secondary_text, -1);
		secondary_markup = g_strdup_printf ("<small>%s</small>",
		                                    escaped);
		g_free (escaped);

		secondary_label = gtk_label_new (secondary_markup);
		g_free (secondary_markup);
		gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
		GTK_WIDGET_SET_FLAGS (secondary_label, GTK_CAN_FOCUS);
		gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
		gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
		gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
		gtk_misc_set_alignment (GTK_MISC (secondary_label), 0, 0.5);
	}

	if (progress)
	{
		GtkWidget *prg = gtk_progress_bar_new ();
		gtk_widget_show (prg);

		gtk_box_pack_start (GTK_BOX (vbox), prg, TRUE, TRUE, 0);
		message_area->priv->progress = prg;
	}

	gtk_widget_show_all (hbox_content);

	content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (message_area));
	gtk_container_add (GTK_CONTAINER (content_area), hbox_content);
}

static void
gedit_collaboration_document_message_class_init (GeditCollaborationDocumentMessageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_document_message_finalize;

	g_type_class_add_private (object_class, sizeof(GeditCollaborationDocumentMessagePrivate));
}

static void
gedit_collaboration_document_message_init (GeditCollaborationDocumentMessage *self)
{
	self->priv = GEDIT_COLLABORATION_DOCUMENT_MESSAGE_GET_PRIVATE (self);
}

gchar *
gedit_collaboration_document_message_error_string (const GError *error)
{
	if (error->domain == GEDIT_COLLABORATION_ERROR)
	{
		switch (error->code)
		{
			case GEDIT_COLLABORATION_ERROR_SESSION_CLOSED:
				return g_strdup (_("The collaboration session for this file was closed"));
			break;
			default:
			break;
		}
	}
	else if (error->domain == inf_request_error_quark ())
	{
		return g_strdup (inf_request_strerror (error->code));
	}
	else if (error->domain == inf_user_error_quark ())
	{
		return g_strdup (inf_user_strerror (error->code));
	}
	else if (error->domain == inf_directory_error_quark ())
	{
		return g_strdup (inf_directory_strerror (error->code));
	}
	else if (error->domain == inf_adopted_state_vector_error_quark ())
	{
		switch (error->code)
		{
			case INF_ADOPTED_STATE_VECTOR_BAD_FORMAT:
				return g_strdup (_("State vector has a bad format"));
			break;
			case INF_ADOPTED_STATE_VECTOR_FAILED:
				return g_strdup (_("State vector failed"));
			break;
			default:
			break;
		}
	}
	else if (error->domain == g_quark_from_static_string ("INF_ADOPTED_SESSION_ERROR"))
	{
		switch (error->code)
		{
			case INF_ADOPTED_SESSION_ERROR_NO_SUCH_USER:
				return g_strdup (_("Adopted session user does not exist"));
			break;
			case INF_ADOPTED_SESSION_ERROR_MISSING_OPERATION:
				return g_strdup (_("Adopted session missing operation"));
			break;
			case INF_ADOPTED_SESSION_ERROR_INVALID_REQUEST:
				return g_strdup (_("Adopted session invalid request"));
			break;
			case INF_ADOPTED_SESSION_ERROR_MISSING_STATE_VECTOR:
				return g_strdup (_("Adopted session missing state vector"));
			break;
			case INF_ADOPTED_SESSION_ERROR_FAILED:
				return g_strdup (_("Adopted session failed"));
			break;
			default:
			break;
		}
	}

	return g_strdup (_("An unknown error occurred"));
}

GtkWidget *
gedit_collaboration_document_message_new_error (const GError *error)
{
	GeditCollaborationDocumentMessage *ret;

	ret = g_object_new (GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, NULL);

	gchar *message = gedit_collaboration_document_message_error_string (error);
	set_message_area_text_and_icon (ret,
	                                GTK_STOCK_DIALOG_ERROR,
	                                message,
	                                error->message,
	                                FALSE);

	g_free (message);

	gtk_info_bar_add_button (GTK_INFO_BAR (ret),
	                         GTK_STOCK_CLOSE,
	                         GTK_RESPONSE_CLOSE);

	return GTK_WIDGET (ret);
}

GtkWidget *
gedit_collaboration_document_message_new_progress (const gchar *primary,
                                                   const gchar *secondary)
{
	GeditCollaborationDocumentMessage *ret;

	ret = g_object_new (GEDIT_COLLABORATION_TYPE_DOCUMENT_MESSAGE, NULL);

	set_message_area_text_and_icon (ret,
	                                GTK_STOCK_DIALOG_INFO,
	                                primary,
	                                secondary,
	                                TRUE);

	gtk_info_bar_add_button (GTK_INFO_BAR (ret),
	                         GTK_STOCK_CANCEL,
	                         GTK_RESPONSE_CANCEL);

	return GTK_WIDGET (ret);
}

void
gedit_collaboration_document_message_update (GeditCollaborationDocumentMessage *document_message,
                                             gdouble                            fraction)
{
	g_return_if_fail (GEDIT_COLLABORATION_IS_DOCUMENT_MESSAGE (document_message));
	g_return_if_fail (document_message->priv->progress != NULL);

	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (document_message->priv->progress),
	                               fraction);
}
