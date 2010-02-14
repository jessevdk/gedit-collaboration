/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-bookmark-dialog.h"

#include "gedit-collaboration.h"

#include <config.h>
#include <gedit/gedit-plugin.h>
#include <glib/gi18n-lib.h>

#include "gedit-collaboration-color-button.h"

#define GEDIT_COLLABORATION_BOOKMARK_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_COLLABORATION_TYPE_BOOKMARK_DIALOG, GeditCollaborationBookmarkDialogPrivate))

struct _GeditCollaborationBookmarkDialogPrivate
{
	GeditCollaborationBookmark *bookmark;
	gboolean isnew;

	GtkEntry *entry_name;
	GtkEntry *entry_host;
	GtkEntry *entry_username;
	GtkSpinButton *spin_button_port;
	GeditCollaborationColorButton *color_button_hue;
};

static void buildable_iface_init (GtkBuildableIface *iface);

GEDIT_PLUGIN_DEFINE_TYPE_WITH_CODE (GeditCollaborationBookmarkDialog, \
                                    gedit_collaboration_bookmark_dialog, \
                                    GTK_TYPE_DIALOG,\
                                    G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init));

static GtkBuildableIface parent_iface;

static gboolean
check_entry (GtkEntry *entry)
{
	const gchar *text = gtk_entry_get_text (entry);
	gboolean empty = !*text;

	if (empty)
	{
		gtk_entry_set_icon_from_stock (entry,
		                               GTK_ENTRY_ICON_SECONDARY,
		                               GTK_STOCK_DIALOG_ERROR);
		return FALSE;
	}

	gtk_entry_set_icon_from_stock (entry, GTK_ENTRY_ICON_SECONDARY, NULL);
	return TRUE;
}

static gboolean
check_fields (GeditCollaborationBookmarkDialog *dialog)
{
	gboolean ret = TRUE;

	ret &= check_entry (dialog->priv->entry_name);
	ret &= check_entry (dialog->priv->entry_host);
	ret &= check_entry (dialog->priv->entry_username);

	return ret;
}

static void
update_bookmark (GeditCollaborationBookmarkDialog *dialog)
{
	GeditCollaborationUser *user;

	user = gedit_collaboration_bookmark_get_user (dialog->priv->bookmark);

	gedit_collaboration_bookmark_set_name (dialog->priv->bookmark,
	                                       gtk_entry_get_text (dialog->priv->entry_name));

	gedit_collaboration_bookmark_set_host (dialog->priv->bookmark,
	                                       gtk_entry_get_text (dialog->priv->entry_host));

	gedit_collaboration_bookmark_set_port (dialog->priv->bookmark,
	                                       gtk_spin_button_get_value (dialog->priv->spin_button_port));

	gedit_collaboration_user_set_name (user,
	                                   gtk_entry_get_text (dialog->priv->entry_username));

	gedit_collaboration_user_set_hue (user,
	                                  gedit_collaboration_color_button_get_hue (dialog->priv->color_button_hue));
}

static void
buildable_parser_finished (GtkBuildable *buildable,
                           GtkBuilder   *builder)
{
	GeditCollaborationBookmarkDialog *dialog;

	dialog = GEDIT_COLLABORATION_BOOKMARK_DIALOG (buildable);

	if (parent_iface.parser_finished)
	{
		parent_iface.parser_finished (buildable, builder);
	}

	dialog->priv->entry_name = GTK_ENTRY (gtk_builder_get_object (builder, "entry_name"));
	dialog->priv->entry_host = GTK_ENTRY (gtk_builder_get_object (builder, "entry_host"));
	dialog->priv->entry_username = GTK_ENTRY (gtk_builder_get_object (builder, "entry_username"));

	dialog->priv->color_button_hue = GEDIT_COLLABORATION_COLOR_BUTTON (gtk_builder_get_object (builder, "color_button_hue"));
	dialog->priv->spin_button_port = GTK_SPIN_BUTTON (gtk_builder_get_object (builder, "spin_button_port"));
}

static void
buildable_iface_init (GtkBuildableIface *iface)
{
	parent_iface = *iface;

	iface->parser_finished = buildable_parser_finished;
}

static void
gedit_collaboration_bookmark_dialog_finalize (GObject *object)
{
	GeditCollaborationBookmarkDialog *dialog = GEDIT_COLLABORATION_BOOKMARK_DIALOG (object);

	g_object_unref (dialog->priv->bookmark);

	G_OBJECT_CLASS (gedit_collaboration_bookmark_dialog_parent_class)->finalize (object);
}

static void
dialog_response_impl (GtkDialog *dlg,
                      gint       response_id)
{
	GeditCollaborationBookmarkDialog *dialog;
	GeditCollaborationBookmarks *bookmarks;

	dialog = GEDIT_COLLABORATION_BOOKMARK_DIALOG (dlg);

	if (response_id == 1)
	{
		/* Reset the port and color to the defaults */
		GeditCollaborationUser *user;

		user = gedit_collaboration_user_get_default ();
		gedit_collaboration_color_button_set_hue (dialog->priv->color_button_hue,
		                                          gedit_collaboration_user_get_hue (user));

		gtk_spin_button_set_value (dialog->priv->spin_button_port, DEFAULT_INFINOTE_PORT);

		gtk_entry_set_text (dialog->priv->entry_username,
		                    gedit_collaboration_user_get_name (user));
		return;
	}

	if (dialog->priv->isnew && response_id != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}

	if (!check_fields (dialog))
	{
		return;
	}

	update_bookmark (dialog);

	bookmarks = gedit_collaboration_bookmarks_get_default ();

	if (dialog->priv->isnew)
	{
		gedit_collaboration_bookmarks_add (bookmarks, dialog->priv->bookmark);
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
gedit_collaboration_bookmark_dialog_class_init (GeditCollaborationBookmarkDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

	object_class->finalize = gedit_collaboration_bookmark_dialog_finalize;
	dialog_class->response = dialog_response_impl;

	g_type_class_add_private (object_class, sizeof(GeditCollaborationBookmarkDialogPrivate));
}

static void
gedit_collaboration_bookmark_dialog_init (GeditCollaborationBookmarkDialog *self)
{
	self->priv = GEDIT_COLLABORATION_BOOKMARK_DIALOG_GET_PRIVATE (self);
}

static GeditCollaborationBookmarkDialog *
create_dialog (const gchar *data_dir)
{
	GtkBuilder *builder;

	builder = gedit_collaboration_create_builder (data_dir,
	                                              "gedit-collaboration-bookmark-dialog.ui");

	if (!builder)
	{
		return NULL;
	}

	return GEDIT_COLLABORATION_BOOKMARK_DIALOG (gtk_builder_get_object (builder, "dialog_bookmark"));
}

static void
set_bookmark (GeditCollaborationBookmarkDialog *dialog,
              GeditCollaborationBookmark       *bookmark)
{
	const gchar *text;
	GeditCollaborationUser *user;

	dialog->priv->bookmark = g_object_ref (bookmark);

	text = gedit_collaboration_bookmark_get_name (bookmark);
	gtk_entry_set_text (dialog->priv->entry_name, text ? text : "");

	text = gedit_collaboration_bookmark_get_host (bookmark);
	gtk_entry_set_text (dialog->priv->entry_host, text ? text : "");

	gtk_spin_button_set_value (dialog->priv->spin_button_port,
	                           gedit_collaboration_bookmark_get_port (bookmark));

	user = gedit_collaboration_bookmark_get_user (bookmark);

	text = gedit_collaboration_user_get_name (user);
	gtk_entry_set_text (dialog->priv->entry_username, text ? text : "");

	gedit_collaboration_color_button_set_hue (dialog->priv->color_button_hue,
	                                          gedit_collaboration_user_get_hue (user));
}

GtkWidget *
gedit_collaboration_bookmark_dialog_new (const gchar                *data_dir,
                                         GeditCollaborationBookmark *bookmark)
{
	GtkWidget *ret = GTK_WIDGET (create_dialog (data_dir));
	GeditCollaborationBookmarkDialog *dialog = GEDIT_COLLABORATION_BOOKMARK_DIALOG (ret);

	gtk_dialog_add_button (GTK_DIALOG (ret),
	                       _("Defaults"),
	                       1);

	if (bookmark == NULL)
	{
		gtk_dialog_add_button (GTK_DIALOG (ret),
		                       GTK_STOCK_CANCEL,
		                       GTK_RESPONSE_CANCEL);

		gtk_dialog_add_button (GTK_DIALOG (ret),
		                       GTK_STOCK_SAVE,
		                       GTK_RESPONSE_OK);

		gtk_window_set_title (GTK_WINDOW (ret), _("Create New Bookmark"));
		gtk_window_set_icon_name (GTK_WINDOW (ret), "bookmark-new");

		bookmark = gedit_collaboration_bookmark_new ();
		dialog->priv->isnew = TRUE;
	}
	else
	{
		gtk_dialog_add_button (GTK_DIALOG (dialog),
		                       GTK_STOCK_CLOSE,
		                       GTK_RESPONSE_CLOSE);

		gtk_window_set_title (GTK_WINDOW (dialog), _("Bookmark Properties"));
		gtk_window_set_icon_name (GTK_WINDOW (dialog), GTK_STOCK_PROPERTIES);

		dialog->priv->isnew = FALSE;
	}

	set_bookmark (dialog, bookmark);

	if (dialog->priv->isnew)
	{
		g_object_unref (bookmark);
	}

	return ret;
}
