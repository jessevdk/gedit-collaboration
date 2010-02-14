/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "gedit-collaboration-window-helper.h"
#include "gedit-collaboration-bookmarks.h"
#include "gedit-collaboration-bookmark-dialog.h"

#include "gedit-collaboration-window-helper-private.h"
#include "gedit-collaboration.h"

#include <libinfgtk/inf-gtk-browser-model-sort.h>

#define XML_UI_FILE "gedit-collaboration-window-helper.ui"
#define DIALOG_BUILDER_KEY "GeditCollaborationBookmarkDialogKey"

#define GEDIT_COLLABORATION_WINDOW_HELPER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_COLLABORATION_WINDOW_HELPER, GeditCollaborationWindowHelperPrivate))

/* Properties */
enum
{
	PROP_0,
	PROP_WINDOW,
	PROP_DATA_DIR
};

GEDIT_PLUGIN_DEFINE_TYPE (GeditCollaborationWindowHelper, gedit_collaboration_window_helper,
                          G_TYPE_OBJECT)

static void
gedit_collaboration_window_helper_finalize (GObject *object)
{
	GeditCollaborationWindowHelper *helper;
	GeditCollaborationBookmarks *bookmarks;

	helper = GEDIT_COLLABORATION_WINDOW_HELPER (object);
	bookmarks = gedit_collaboration_bookmarks_get_default ();

	if (helper->priv->added_handler_id)
	{
		g_signal_handler_disconnect (bookmarks, helper->priv->added_handler_id);
	}

	if (helper->priv->removed_handler_id)
	{
		g_signal_handler_disconnect (bookmarks, helper->priv->removed_handler_id);
	}

	if (helper->priv->io)
	{
		g_object_unref (helper->priv->io);
	}

	if (helper->priv->certificate_credentials)
	{
		inf_certificate_credentials_unref (helper->priv->certificate_credentials);
	}

	if (helper->priv->builder)
	{
		g_object_unref (helper->priv->builder);
	}

	G_OBJECT_CLASS (gedit_collaboration_window_helper_parent_class)->finalize (object);
}

static void
gedit_collaboration_window_helper_dispose (GObject *object)
{
	GeditCollaborationWindowHelper *helper = GEDIT_COLLABORATION_WINDOW_HELPER (object);

	if (helper->priv->window)
	{
		if (helper->priv->ui_id > 0)
		{
			GtkUIManager *manager;

			manager = gedit_window_get_ui_manager (helper->priv->window);
			gtk_ui_manager_remove_ui (manager, helper->priv->ui_id);
		}

		if (helper->priv->panel_widget)
		{
			GeditPanel *panel;

			panel = gedit_window_get_side_panel (helper->priv->window);
			gedit_panel_remove_item (panel, helper->priv->panel_widget);
		}

		g_object_unref (helper->priv->window);
		helper->priv->window = NULL;
	}

	if (helper->priv->manager)
	{
		g_object_unref (helper->priv->manager);
		helper->priv->manager = NULL;
	}
}

static void
gedit_collaboration_window_helper_set_property (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *pspec)
{
	GeditCollaborationWindowHelper *self = GEDIT_COLLABORATION_WINDOW_HELPER (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			if (self->priv->window)
			{
				g_object_unref (self->priv->window);
			}

			self->priv->window = g_value_dup_object (value);
		break;
		case PROP_DATA_DIR:
			g_free (self->priv->data_dir);
			self->priv->data_dir = g_value_dup_string (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_window_helper_get_property (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *pspec)
{
	GeditCollaborationWindowHelper *self = GEDIT_COLLABORATION_WINDOW_HELPER (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, self->priv->window);
		break;
		case PROP_DATA_DIR:
			g_value_set_string (value, self->priv->data_dir);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static GtkActionGroup *
get_action_group (GeditCollaborationWindowHelper *helper,
                  const gchar                    *id)
{
	return GTK_ACTION_GROUP (gtk_builder_get_object (helper->priv->builder, id));
}

static GtkAction *
get_action (GeditCollaborationWindowHelper *helper,
            const gchar                    *id)
{
	return GTK_ACTION (gtk_builder_get_object (helper->priv->builder, id));
}

static void
update_sensitivity (GeditCollaborationWindowHelper *helper)
{
	gboolean has_selection;
	GtkTreeIter selected;
	GtkTreeIter sorted;
	InfcBrowser *browser = NULL;
	InfDiscovery *discovery = NULL;
	GtkTreeModel *model;
	GtkAction *action;
	GtkTreePath *path;
	gboolean toplevel = FALSE;

	model = GTK_TREE_MODEL (helper->priv->browser_store);

	has_selection =
		inf_gtk_browser_view_get_selected (INF_GTK_BROWSER_VIEW (helper->priv->browser_view),
		                                   &sorted);

	if (has_selection)
	{
		gtk_tree_model_sort_convert_iter_to_child_iter (
			GTK_TREE_MODEL_SORT (
				inf_gtk_browser_view_get_model (
					INF_GTK_BROWSER_VIEW (helper->priv->browser_view)
				)
			),
			&selected,
			&sorted
		);
	}

	if (has_selection)
	{
		/* Check if the iter is a top-level (aka. browser node) */
		gtk_tree_model_get (model,
		                    &selected,
		                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
		                    &browser,
		                    INF_GTK_BROWSER_MODEL_COL_DISCOVERY,
		                    &discovery,
		                    -1);

		path = gtk_tree_model_get_path (model, &selected);
		toplevel = gtk_tree_path_get_depth (path) == 1;
		gtk_tree_path_free (path);
	}

	gtk_action_group_set_sensitive (get_action_group (helper, "action_group_connected"),
	                                browser != NULL &&
	                                infc_browser_get_status (browser) == INFC_BROWSER_CONNECTED);

	action = get_action (helper, "SessionDisconnect");
	gtk_action_set_sensitive (action,
	                          toplevel &&
	                          browser != NULL &&
	                          infc_browser_get_status (browser) != INFC_BROWSER_DISCONNECTED);

	/* Handle other actions manually */
	action = get_action (helper, "ItemDelete");
	gtk_action_set_sensitive (action,
	                          has_selection &&
	                          (!toplevel || (
	                          discovery == NULL &&
	                          (browser == NULL ||
	                           infc_browser_get_status (browser) != INFC_BROWSER_CONNECTED))));


	action = get_action (helper, "BookmarkEdit");
	gtk_action_set_sensitive (action,
	                          has_selection &&
	                          discovery == NULL &&
	                          toplevel);

	if (browser)
	{
		g_object_unref (browser);
	}

	if (discovery)
	{
		g_object_unref (discovery);
	}
}

static void
on_selection_changed (InfGtkBrowserView              *browser_view,
                      GtkTreeIter                    *iter,
                      GeditCollaborationWindowHelper *helper)
{
	update_sensitivity (helper);
}

static void
on_browser_status_changed (InfcBrowser                    *browser,
                           GParamSpec                     *spec,
                           GeditCollaborationWindowHelper *helper)
{
	update_sensitivity (helper);
}

static void
on_set_browser (InfGtkBrowserModel             *model,
                GtkTreePath                    *path,
                GtkTreeIter                    *iter,
                InfcBrowser                    *browser,
                GeditCollaborationWindowHelper *helper)
{
	if (browser != NULL)
	{
		infc_browser_add_plugin (browser,
		                         gedit_collaboration_manager_get_note_plugin (helper->priv->manager));

		g_signal_connect (browser,
		                  "notify::status",
		                  G_CALLBACK (on_browser_status_changed),
		                  helper);
	}

	update_sensitivity (helper);
}

typedef struct
{
	GeditCollaborationWindowHelper *helper;
	InfXmlConnection *connection;
} NameInfo;

static void
on_bookmark_name_changed (GeditCollaborationBookmark *bookmark,
                          GParamSpec                 *spec,
                          NameInfo                   *info)
{
	inf_gtk_browser_store_set_connection_name (info->helper->priv->browser_store,
	                                           info->connection,
	                                           gedit_collaboration_bookmark_get_name (bookmark));
}

static void
name_info_free (NameInfo *info)
{
	g_slice_free (NameInfo, info);
}

static gchar *
show_password_dialog (GeditCollaborationWindowHelper *helper,
                      GeditCollaborationUser         *user,
                      InfXmppConnection              *connection)
{
	GtkBuilder *builder;
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *entry;
	gchar *password;
	gchar *remote;
	gchar *text;
	gchar *name;

	builder = gedit_collaboration_create_builder (helper->priv->data_dir,
	                                              "gedit-collaboration-password-dialog.ui");

	if (!builder)
	{
		return NULL;
	}

	dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dialog_password"));
	label = GTK_WIDGET (gtk_builder_get_object (builder, "label_caption"));
	entry = GTK_WIDGET (gtk_builder_get_object (builder, "entry_password"));

	g_object_get (connection, "remote-hostname", &remote, NULL);

	name = g_strdup_printf ("<i>%s@%s</i>",
	                       gedit_collaboration_user_get_name (user),
	                       remote);
	g_free (remote);

	text = g_strdup_printf (_("Please provide a password for %s"),
	                        name);
	g_free (name);

	gtk_label_set_markup (GTK_LABEL (label),
	                      text);
	g_free (text);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	/* Need to do this modal/sync for now */
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
	{
		password = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

		if (!*password)
		{
			g_free (password);
			password = NULL;
		}
	}
	else
	{
		password = NULL;
	}

	g_object_unref (builder);
	gtk_widget_destroy (dialog);

	return password;
}

static void
user_request_password (GeditCollaborationUser         *user,
                       gpointer                        session_data,
                       GeditCollaborationWindowHelper *helper)
{
	gchar *password;

	password = show_password_dialog (helper, user, session_data);
	gedit_collaboration_user_set_password (user, password);

	g_free (password);
}

static void
bookmark_added (GeditCollaborationWindowHelper *helper,
                GeditCollaborationBookmark     *bookmark)
{
	GResolver *resolver = g_resolver_get_default ();
	GList *addresses;
	InfTcpConnection *tcp;
	InfIpAddress *ipaddress;
	InfXmppConnection *connection;
	gchar *ipaddr;
	NameInfo *info;
	GeditCollaborationUser *user;

	/* TODO: make this asynchronous and be smarter about it */
	addresses = g_resolver_lookup_by_name (resolver,
	                                       gedit_collaboration_bookmark_get_host (bookmark),
	                                       NULL,
	                                       NULL);

	if (!addresses)
	{
		return;
	}

	ipaddr = g_inet_address_to_string ((GInetAddress *)addresses->data);
	g_resolver_free_addresses (addresses);

	ipaddress = inf_ip_address_new_from_string (ipaddr);
	g_free (ipaddr);

	tcp = inf_tcp_connection_new (helper->priv->io,
	                              ipaddress,
	                              (guint)gedit_collaboration_bookmark_get_port (bookmark));

	user = gedit_collaboration_bookmark_get_user (bookmark);
	connection = inf_xmpp_connection_new (tcp,
	                                      INF_XMPP_CONNECTION_CLIENT,
	                                      NULL,
	                                      gedit_collaboration_bookmark_get_host (bookmark),
	                                      INF_XMPP_CONNECTION_SECURITY_BOTH_PREFER_TLS,
	                                      helper->priv->certificate_credentials,
	                                      gedit_collaboration_user_get_sasl_context (user),
	                                      "ANONYMOUS PLAIN");

	g_signal_connect (user,
	                  "request-password",
	                  G_CALLBACK (user_request_password),
	                  helper);

	inf_gtk_browser_store_add_connection (helper->priv->browser_store,
	                                      INF_XML_CONNECTION (connection),
	                                      gedit_collaboration_bookmark_get_name (bookmark));

	g_object_set_data (G_OBJECT (connection), BOOKMARK_DATA_KEY, bookmark);

	inf_ip_address_free (ipaddress);
	g_object_unref (tcp);

	info = g_slice_new (NameInfo);
	info->helper = helper;
	info->connection = INF_XML_CONNECTION (connection);

	g_signal_connect_data (bookmark,
	                       "notify::name",
	                       G_CALLBACK (on_bookmark_name_changed),
	                       info,
	                       (GClosureNotify)name_info_free,
	                       0);
}

static void
on_bookmark_added (GeditCollaborationBookmarks    *bookmarks,
                   GeditCollaborationBookmark     *bookmark,
                   GeditCollaborationWindowHelper *helper)
{
	bookmark_added (helper, bookmark);
}

static void
on_bookmark_removed (GeditCollaborationBookmarks    *bookmarks,
                     GeditCollaborationBookmark     *bookmark,
                     GeditCollaborationWindowHelper *helper)
{

}

static void
init_bookmarks (GeditCollaborationWindowHelper *helper)
{
	GeditCollaborationBookmarks *bookmarks;
	GList *item;

	bookmarks = gedit_collaboration_bookmarks_get_default ();
	item = gedit_collaboration_bookmarks_get_bookmarks (bookmarks);

	while (item)
	{
		GeditCollaborationBookmark *bookmark = item->data;

		bookmark_added (helper, bookmark);
		item = g_list_next (item);
	}

	helper->priv->added_handler_id =
		g_signal_connect (bookmarks,
		                  "added",
		                  G_CALLBACK (on_bookmark_added),
		                  helper);

	helper->priv->removed_handler_id =
		g_signal_connect (bookmarks,
		                  "removed",
		                  G_CALLBACK (on_bookmark_removed),
		                  helper);
}

static gboolean
create_popup_menu_item (GeditCollaborationWindowHelper *helper,
                        GtkMenu                        *menu,
                        const gchar                    *id,
                        gboolean                        create_separator)
{
	GtkAction *action;
	GtkWidget *item;
	GtkActionGroup *ac;

	action = get_action (helper, id);
	g_object_get (action, "action-group", &ac, NULL);

	if (!gtk_action_get_sensitive (action) ||
	    !gtk_action_group_get_sensitive (ac))
	{
		g_object_unref (ac);
		return FALSE;
	}

	gtk_action_set_accel_group (action,
	                            gtk_ui_manager_get_accel_group (helper->priv->uimanager));

	g_object_unref (ac);

	if (create_separator)
	{
		item = gtk_separator_menu_item_new ();
		gtk_widget_show (item);

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	}

	item = gtk_action_create_menu_item (action);
	gtk_widget_show (item);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	return TRUE;
}

static void
on_populate_popup (InfGtkBrowserView              *view,
                   GtkMenu                        *menu,
                   GeditCollaborationWindowHelper *helper)
{
	gboolean createsep;

	create_popup_menu_item (helper, menu, "FileNew", FALSE);
	createsep = create_popup_menu_item (helper, menu, "FolderNew", FALSE);

	createsep &= !create_popup_menu_item (helper, menu, "ItemDelete", createsep);
	createsep |= create_popup_menu_item (helper, menu, "SessionDisconnect", createsep);

	createsep &= !create_popup_menu_item (helper, menu, "BookmarkNew", createsep);
	create_popup_menu_item (helper, menu, "BookmarkEdit", createsep);
}

static void
on_browser_activate (InfGtkBrowserView              *view,
                     GtkTreeIter                    *iter,
                     GeditCollaborationWindowHelper *helper)
{
	InfcBrowser *browser;
	InfcBrowserIter *browser_iter;
	InfDiscovery *discovery;
	GeditCollaborationUser *user;
	GtkTreeIter selected;

	gtk_tree_model_sort_convert_iter_to_child_iter (
		GTK_TREE_MODEL_SORT (inf_gtk_browser_view_get_model (view)),
		&selected,
		iter
	);

	gtk_tree_model_get (GTK_TREE_MODEL (helper->priv->browser_store),
	                    &selected,
	                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
	                    &browser,
	                    INF_GTK_BROWSER_MODEL_COL_DISCOVERY,
	                    &discovery,
	                    -1);

	if (browser == NULL)
	{
		if (discovery)
		{
			g_object_unref (discovery);
		}

		return;
	}

	gtk_tree_model_get (GTK_TREE_MODEL (helper->priv->browser_store),
	                    &selected,
	                    INF_GTK_BROWSER_MODEL_COL_NODE,
	                    &browser_iter,
	                    -1);

	if (browser_iter == NULL ||
	    infc_browser_iter_is_subdirectory (browser, browser_iter))
	{
		g_object_unref (browser);

		if (discovery)
		{
			g_object_unref (discovery);
		}

		return;
	}

	if (discovery)
	{
		user = gedit_collaboration_user_get_default ();
	}
	else
	{
		GeditCollaborationBookmark *bookmark;

		bookmark = g_object_get_data (G_OBJECT (infc_browser_get_connection (browser)),
		                              BOOKMARK_DATA_KEY);

		user = gedit_collaboration_bookmark_get_user (bookmark);
	}

	gedit_collaboration_manager_subscribe (helper->priv->manager,
	                                       user,
	                                       browser,
	                                       browser_iter);

	if (discovery)
	{
		g_object_unref (discovery);
	}
}

#ifdef LIBINFINITY_HAVE_AVAHI
static void
init_infinity_discovery (GeditCollaborationWindowHelper *helper,
                         InfXmppManager                 *xmpp_manager)
{
	InfDiscoveryAvahi *discovery;
	GeditCollaborationUser *user;

	user = gedit_collaboration_user_get_default ();
	discovery = inf_discovery_avahi_new (helper->priv->io,
	                                     xmpp_manager,
	                                     helper->priv->certificate_credentials,
	                                     gedit_collaboration_user_get_sasl_context (user),
	                                     "ANONYMOUS PLAIN");

	g_signal_connect (user,
	                 "request-password",
	                 G_CALLBACK (user_request_password),
	                 helper);

	inf_gtk_browser_store_add_discovery (helper->priv->browser_store,
	                                     INF_DISCOVERY (discovery));
}
#endif

/* Copied from gobby: code/core/browser.cpp */
static gint
compare_func (GtkTreeModel *model,
              GtkTreeIter  *first,
              GtkTreeIter  *second,
              gpointer      user_data)
{
	gint result;
	InfcBrowser *br_one;
	InfcBrowser *br_two;
	InfcBrowserIter *bri_one;
	InfcBrowserIter *bri_two;
	GtkTreeIter parent;

	result = 0;

	if (gtk_tree_model_iter_parent (model, &parent, first))
	{
		g_assert (gtk_tree_model_iter_parent (model, &parent, second));

		gtk_tree_model_get (model,
		                   first,
		                   INF_GTK_BROWSER_MODEL_COL_BROWSER,
		                   &br_one,
		                   INF_GTK_BROWSER_MODEL_COL_NODE,
		                   &bri_one,
		                   -1);

		gtk_tree_model_get (model,
		                    second,
		                    INF_GTK_BROWSER_MODEL_COL_BROWSER,
		                    &br_two,
		                    INF_GTK_BROWSER_MODEL_COL_NODE,
		                    &bri_two,
		                    -1);

		if (infc_browser_iter_is_subdirectory (br_one, bri_one) &&
		   !infc_browser_iter_is_subdirectory (br_two, bri_two))
		{
			result = -1;
		}
		else if (!infc_browser_iter_is_subdirectory(br_one, bri_one) &&
		          infc_browser_iter_is_subdirectory(br_two, bri_two))
		{
			result = 1;
		}

		g_object_unref (br_one);
		g_object_unref (br_two);

		infc_browser_iter_free (bri_one);
		infc_browser_iter_free (bri_two);
	}

	if (!result)
	{
		gchar* name_one;
		gchar* name_two;

		gtk_tree_model_get (model,
		                    first,
		                    INF_GTK_BROWSER_MODEL_COL_NAME,
		                    &name_one,
		                    -1);

		gtk_tree_model_get (model,
		                    second,
		                    INF_GTK_BROWSER_MODEL_COL_NAME,
		                    &name_two,
		                    -1);

		gchar* one = g_utf8_casefold (name_one, -1);
		gchar* two = g_utf8_casefold (name_two, -1);

		result = g_utf8_collate (one, two);

		g_free (name_one);
		g_free (name_two);
		g_free (one);
		g_free (two);
	}

	return result;
}

static void
init_infinity (GeditCollaborationWindowHelper *helper)
{
	InfGtkIo *io;
	InfCommunicationManager *communication_manager;
	InfXmppManager *xmpp_manager;
	InfCertificateCredentials *certificate_credentials;
	InfGtkBrowserModel *model_sort;

	io = inf_gtk_io_new ();
	communication_manager = inf_communication_manager_new ();
	xmpp_manager = inf_xmpp_manager_new ();
	certificate_credentials = inf_certificate_credentials_new ();

	helper->priv->io = INF_IO (io);
	helper->priv->certificate_credentials = certificate_credentials;
	helper->priv->browser_store = inf_gtk_browser_store_new (INF_IO (io),
	                                                         communication_manager);

	model_sort = INF_GTK_BROWSER_MODEL (
		inf_gtk_browser_model_sort_new (
			INF_GTK_BROWSER_MODEL (helper->priv->browser_store)
		)
	);

	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (model_sort),
	                                         compare_func,
	                                         NULL,
	                                         NULL);

	helper->priv->browser_view =
		inf_gtk_browser_view_new_with_model (model_sort);

	gtk_widget_show (helper->priv->browser_view);

	g_signal_connect_after (helper->priv->browser_store,
	                        "set-browser",
	                        G_CALLBACK (on_set_browser),
	                        helper);

	g_signal_connect (helper->priv->browser_view,
	                  "selection-changed",
	                  G_CALLBACK (on_selection_changed),
	                  helper);

	g_signal_connect (helper->priv->browser_view,
	                  "populate-popup",
	                  G_CALLBACK (on_populate_popup),
	                  helper);

	g_signal_connect (helper->priv->browser_view,
	                  "activate",
	                  G_CALLBACK (on_browser_activate),
	                  helper);

#ifdef LIBINFINITY_HAVE_AVAHI
	init_infinity_discovery (helper, xmpp_manager);
#endif

	init_bookmarks (helper);

	g_object_unref (communication_manager);
	g_object_unref (xmpp_manager);
}

static void
on_clear_collaboration_colors_activate (GtkAction                      *action,
                                        GeditCollaborationWindowHelper *helper)
{
	GeditTab *tab;

	tab = gedit_window_get_active_tab (helper->priv->window);

	gedit_collaboration_manager_clear_colors (helper->priv->manager,
	                                          tab);
}

static const gchar submenu[] = {
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu name='ViewMenu' action='View'>"
"      <separator />"
"      <menuitem name='CollaborationClearColors' action='CollaborationClearColorsAction'/>"
"    </menu>"
"  </menubar>"
"</ui>"
};

static const GtkActionEntry action_clear_colors_entries[] =
{
	{ "CollaborationClearColorsAction", NULL, N_("Clear _Collaboration Colors"), NULL,
	 N_("Clear collaboration user colors"),
	 G_CALLBACK (on_clear_collaboration_colors_activate)},
};

static void
add_window_menu (GeditCollaborationWindowHelper *helper)
{
	GtkUIManager *manager;
	manager = gedit_window_get_ui_manager (helper->priv->window);

	helper->priv->action_group = gtk_action_group_new ("GeditCollaborationWindowActions");
	gtk_action_group_set_translation_domain (helper->priv->action_group,
	                                         GETTEXT_PACKAGE);

	gtk_action_group_add_actions (helper->priv->action_group,
	                              action_clear_colors_entries,
	                              G_N_ELEMENTS (action_clear_colors_entries),
	                              helper);

	gtk_ui_manager_insert_action_group (manager, helper->priv->action_group, -1);

	helper->priv->ui_id = gtk_ui_manager_add_ui_from_string (manager,
	                                                         submenu,
	                                                         -1,
	                                                         NULL);
}

static GdkPixbuf *
try_create_icon (const gchar *data_dir)
{
	gchar *path;
	gint width, height;
	GdkPixbuf *icon;

	path = g_build_filename (data_dir, "icons", "people.svg", NULL);

	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
	icon = gdk_pixbuf_new_from_file_at_size (path, width, height, NULL);
	g_free (path);

	return icon;
}

static gboolean
build_ui (GeditCollaborationWindowHelper *helper)
{
	GeditPanel *panel;
	GtkWidget *vbox;
	GtkWidget *sw;
	GtkWidget *toolbar;
	GtkWidget *image;
	GdkPixbuf *icon;
	GtkBuilder *builder;

	builder = gedit_collaboration_create_builder (helper->priv->data_dir,
	                                              XML_UI_FILE);

	if (!builder)
	{
		return FALSE;
	}

	helper->priv->builder = builder;
	helper->priv->uimanager = GTK_UI_MANAGER (gtk_builder_get_object (builder, "uimanager"));

	gtk_builder_connect_signals (builder, helper);

	/* Create panel */
	panel = gedit_window_get_side_panel (helper->priv->window);

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vbox);

	toolbar = gtk_ui_manager_get_widget (helper->priv->uimanager, "/ToolBar");
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (toolbar), GTK_ICON_SIZE_MENU);
	gtk_widget_show (toolbar);

	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (sw);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
	                                     GTK_SHADOW_ETCHED_IN);

	/* Initialize libinfinity stuff */
	init_infinity (helper);
	gtk_container_add (GTK_CONTAINER (sw), helper->priv->browser_view);

	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

	/* Create collaboration icon */
	icon = try_create_icon (helper->priv->data_dir);

	if (icon == NULL)
	{
		icon = try_create_icon (GEDIT_PLUGINS_DATA_DIR "/collaboration");
	}

	image = gtk_image_new_from_pixbuf (icon);

	if (icon != NULL)
	{
		g_object_unref (icon);
	}

	gtk_widget_show (image);
	gedit_panel_add_item (panel, vbox, _("Collaboration"), image);
	helper->priv->panel_widget = vbox;

	update_sensitivity (helper);

	add_window_menu (helper);
	return TRUE;
}

static GObject *
gedit_collaboration_window_helper_constructor (GType                  type,
                                               guint                  n_construct_params,
                                               GObjectConstructParam *construct_params)
{
	GeditCollaborationWindowHelper *helper;
	GObject *ret;

	ret = G_OBJECT_CLASS (gedit_collaboration_window_helper_parent_class)->constructor (type,
	                                                                                    n_construct_params,
	                                                                                    construct_params);

	helper = GEDIT_COLLABORATION_WINDOW_HELPER (ret);

	helper->priv->manager = gedit_collaboration_manager_new (helper->priv->window);

	if (!build_ui (helper))
	{
		g_object_unref (ret);
		return NULL;
	}

	return ret;
}

static void
gedit_collaboration_window_helper_class_init (GeditCollaborationWindowHelperClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_window_helper_finalize;
	object_class->dispose = gedit_collaboration_window_helper_dispose;

	object_class->set_property = gedit_collaboration_window_helper_set_property;
	object_class->get_property = gedit_collaboration_window_helper_get_property;
	object_class->constructor = gedit_collaboration_window_helper_constructor;

	g_object_class_install_property (object_class,
	                                 PROP_WINDOW,
	                                 g_param_spec_object ("window",
	                                                      "Window",
	                                                      "The gedit window",
	                                                      GEDIT_TYPE_WINDOW,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_DATA_DIR,
	                                 g_param_spec_string ("data-dir",
	                                                      "Data Dir",
	                                                      "Data dir",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private (object_class, sizeof(GeditCollaborationWindowHelperPrivate));
}

static void
gedit_collaboration_window_helper_init (GeditCollaborationWindowHelper *self)
{
	self->priv = GEDIT_COLLABORATION_WINDOW_HELPER_GET_PRIVATE (self);
}

GeditCollaborationWindowHelper *
gedit_collaboration_window_helper_new (GeditWindow *window,
                                       const gchar *data_dir)
{
	return g_object_new (GEDIT_TYPE_COLLABORATION_WINDOW_HELPER,
	                     "window", window,
	                     "data-dir", data_dir,
	                     NULL);
}

void
gedit_collaboration_window_helper_update_ui (GeditCollaborationWindowHelper *helper)
{
	GeditTab *tab;
	gboolean sensitive;
	GtkAction *action;

	g_return_if_fail (GEDIT_IS_COLLABORATION_WINDOW_HELPER (helper));

	tab = gedit_window_get_active_tab (helper->priv->window);

	sensitive = tab != NULL &&
	            gedit_collaboration_manager_tab_is_managed (helper->priv->manager,
	                                                        tab);

	action = gtk_action_group_get_action (helper->priv->action_group,
	                                      "CollaborationClearColorsAction");

	gtk_action_set_sensitive (action, sensitive);
}
