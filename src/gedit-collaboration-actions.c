/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-actions.h"
#include "gedit-collaboration-window-helper-private.h"
#include "gedit-collaboration-bookmarks.h"
#include "gedit-collaboration-bookmark-dialog.h"

typedef struct
{
	GeditCollaborationWindowHelper *helper;
	InfcBrowserIter iter;
	InfcBrowser *browser;
	gboolean newfile;
	GtkEntry *entry;
} ItemNew;

static void
on_item_new_response (GtkDialog *dialog,
                      gint       responseid,
                      ItemNew   *item)
{
	gchar *name = NULL;

	if (responseid == GTK_RESPONSE_OK)
	{
		const gchar *text = gtk_entry_get_text (item->entry);

		if (!*text)
		{
			return;
		}

		name = g_strdup (text);
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));

	if (name)
	{
		InfcBrowserIter parent = item->iter;
		InfcNodeRequest *request;

		if (!infc_browser_iter_is_subdirectory (item->browser, &parent))
		{
			infc_browser_iter_get_parent (item->browser, &parent);
		}

		if (item->newfile)
		{
			InfcNotePlugin *plugin;
			plugin = gedit_collaboration_manager_get_note_plugin (item->helper->priv->manager);

			request = infc_browser_add_note (item->browser,
			                                 &parent,
			                                 name,
			                                 plugin,
			                                 FALSE);
		}
		else
		{
			request = infc_browser_add_subdirectory (item->browser,
			                                         &parent,
			                                         name);
		}
	}

	g_free (name);

	g_object_unref (item->browser);

	g_slice_free (ItemNew, item);
}

static void
item_new_dialog (GeditCollaborationWindowHelper *helper,
                 gboolean                        newfile)
{
	GtkWidget *dialog;
	ItemNew *item;
	GtkTreeIter iter;
	GtkTreeIter selected;
	InfcBrowser *browser;
	InfcBrowserIter *node;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *hbox;

	if (!inf_gtk_browser_view_get_selected (INF_GTK_BROWSER_VIEW (helper->priv->browser_view),
	                                        &selected))
	{
		return;
	}

	gtk_tree_model_sort_convert_iter_to_child_iter (
		GTK_TREE_MODEL_SORT (
			inf_gtk_browser_view_get_model (
				INF_GTK_BROWSER_VIEW (helper->priv->browser_view)
			)
		),
		&iter,
		&selected
	);

	dialog = gtk_dialog_new_with_buttons (newfile ? _("New File") : _("New Folder"),
	                                      GTK_WINDOW (helper->priv->window),
	                                      GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      GTK_STOCK_CANCEL,
	                                      GTK_RESPONSE_CANCEL,
	                                      GTK_STOCK_OK,
	                                      GTK_RESPONSE_OK,
	                                      NULL);

	hbox = gtk_hbox_new (FALSE, 6);
	gtk_widget_show (hbox);

	label = gtk_label_new_with_mnemonic (newfile ? _("File _name:") : _("Folder _name:"));
	gtk_widget_show (label);

	entry = gtk_entry_new ();
	gtk_widget_show (entry);
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);

	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
	                    hbox,
	                    FALSE,
	                    FALSE,
	                    0);

	gtk_tree_model_get (GTK_TREE_MODEL (helper->priv->browser_store),
	                    &iter,
	                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
	                    &browser,
	                    INF_GTK_BROWSER_MODEL_COL_NODE,
	                    &node,
	                    -1);

	item = g_slice_new (ItemNew);
	item->helper = helper;
	item->iter = *node;
	item->browser = browser;
	item->newfile = newfile;
	item->entry = GTK_ENTRY (entry);

	infc_browser_iter_free (node);

	g_signal_connect (dialog,
	                  "response",
	                  G_CALLBACK (on_item_new_response),
	                  item);

	gtk_widget_show (dialog);
}

void
on_action_file_new (GtkAction                      *action,
                    GeditCollaborationWindowHelper *helper)
{
	item_new_dialog (helper, TRUE);
}

void
on_action_folder_new (GtkAction                      *action,
                      GeditCollaborationWindowHelper *helper)
{
	item_new_dialog (helper, FALSE);
}

void
on_action_session_disconnect (GtkAction                      *action,
                              GeditCollaborationWindowHelper *helper)
{
	GtkTreeIter iter;
	GtkTreeIter selected;
	InfcBrowser *browser;
	InfXmlConnection *connection;

	if (!inf_gtk_browser_view_get_selected (INF_GTK_BROWSER_VIEW (helper->priv->browser_view),
	                                        &selected))
	{
		return;
	}

	gtk_tree_model_sort_convert_iter_to_child_iter (
		GTK_TREE_MODEL_SORT (
			inf_gtk_browser_view_get_model (
				INF_GTK_BROWSER_VIEW (helper->priv->browser_view)
			)
		),
		&iter,
		&selected
	);

	gtk_tree_model_get (GTK_TREE_MODEL (helper->priv->browser_store),
	                    &iter,
	                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
	                    &browser,
	                    -1);

	if (browser == NULL)
	{
		return;
	}

	connection = infc_browser_get_connection (browser);
	inf_xml_connection_close (connection);

	inf_gtk_browser_store_clear_connection_error (helper->priv->browser_store,
	                                              connection);

	g_object_unref (browser);
}

void
on_action_item_delete (GtkAction                      *action,
                       GeditCollaborationWindowHelper *helper)
{
	GtkTreeIter iter;
	GtkTreeIter selected;
	InfcBrowser *browser;
	InfcBrowserIter *browser_iter;
	InfcBrowserIter parent;

	if (!inf_gtk_browser_view_get_selected (INF_GTK_BROWSER_VIEW (helper->priv->browser_view),
	                                        &selected))
	{
		return;
	}

	gtk_tree_model_sort_convert_iter_to_child_iter (
		GTK_TREE_MODEL_SORT (
			inf_gtk_browser_view_get_model (
				INF_GTK_BROWSER_VIEW (helper->priv->browser_view)
			)
		),
		&iter,
		&selected
	);

	gtk_tree_model_get (GTK_TREE_MODEL (helper->priv->browser_store),
	                    &iter,
	                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
	                    &browser,
	                    INF_GTK_BROWSER_MODEL_COL_NODE,
	                    &browser_iter,
	                    -1);

	parent = *browser_iter;

	if (!infc_browser_iter_get_parent (browser, &parent))
	{
		/* Toplevel bookmark */
		InfXmlConnection *connection = infc_browser_get_connection (browser);
		GeditCollaborationBookmarks *bookmarks;
		GeditCollaborationBookmark *bookmark =
			g_object_get_data (G_OBJECT (connection),
			                   BOOKMARK_DATA_KEY);

		/* Close connection first */
		if (infc_browser_get_status (browser) != INFC_BROWSER_DISCONNECTED)
		{
			inf_xml_connection_close (connection);
		}

		inf_gtk_browser_store_remove_connection (helper->priv->browser_store,
		                                         connection);

		bookmarks = gedit_collaboration_bookmarks_get_default ();
		gedit_collaboration_bookmarks_remove (bookmarks, bookmark);
	}
	else
	{
		/* Remove the iter itself */
		infc_browser_remove_node (browser, browser_iter);
	}

	g_object_unref (browser);

	if (browser_iter)
	{
		infc_browser_iter_free (browser_iter);
	}
}

static void
create_bookmark_dialog (GeditCollaborationWindowHelper *helper,
                        GeditCollaborationBookmark     *bookmark)
{
	GtkWidget *dialog;

	dialog = gedit_collaboration_bookmark_dialog_new (helper->priv->data_dir,
	                                                  bookmark);

	if (dialog == NULL)
	{
		return;
	}

	gtk_window_set_transient_for (GTK_WINDOW (dialog),
	                              GTK_WINDOW (helper->priv->window));

	gtk_widget_show (dialog);
}

void
on_action_bookmark_new (GtkAction                      *action,
                        GeditCollaborationWindowHelper *helper)
{
	create_bookmark_dialog (helper, NULL);
}

void
on_action_bookmark_edit (GtkAction                      *action,
                         GeditCollaborationWindowHelper *helper)
{
	GtkTreeIter iter;
	InfcBrowser *browser;
	GeditCollaborationBookmark *bookmark;
	InfXmlConnection *connection;
	GtkTreeIter selected;

	if (!inf_gtk_browser_view_get_selected (INF_GTK_BROWSER_VIEW (helper->priv->browser_view),
	                                        &selected))
	{
		return;
	}

	gtk_tree_model_sort_convert_iter_to_child_iter (
		GTK_TREE_MODEL_SORT (
			inf_gtk_browser_view_get_model (
				INF_GTK_BROWSER_VIEW (helper->priv->browser_view)
			)
		),
		&iter,
		&selected
	);

	gtk_tree_model_get (GTK_TREE_MODEL (helper->priv->browser_store),
	                    &iter,
	                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
	                    &browser,
	                    -1);

	if (browser == NULL)
	{
		return;
	}

	connection = infc_browser_get_connection (browser);
	bookmark = g_object_get_data (G_OBJECT (connection),
	                              BOOKMARK_DATA_KEY);

	create_bookmark_dialog (helper, bookmark);

	g_object_unref (browser);
}
