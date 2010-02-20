/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GEDIT_COLLABORATION_WINDOW_HELPER_PRIVATE_H__
#define __GEDIT_COLLABORATION_WINDOW_HELPER_PRIVATE_H__

#include <config.h>
#include <glib/gi18n-lib.h>
#include <gedit/gedit-plugin.h>

#include <libinfgtk/inf-gtk-browser-view.h>
#include <libinfgtk/inf-gtk-browser-store.h>
#include <libinfgtk/inf-gtk-io.h>
#include <libinfinity/common/inf-xmpp-manager.h>
#include <libinfinity/inf-config.h>

#ifdef LIBINFINITY_HAVE_AVAHI
#include <libinfinity/common/inf-discovery-avahi.h>
#endif

#include "gedit-collaboration-manager.h"

#define BOOKMARK_DATA_KEY "GeditCollaborationBookmarkDataKey"

G_BEGIN_DECLS

struct _GeditCollaborationWindowHelperPrivate
{
	GeditWindow *window;
	gchar *data_dir;

	InfIo *io;
	InfCertificateCredentials *certificate_credentials;
	InfGtkBrowserStore *browser_store;
	GtkWidget *browser_view;
	GeditCollaborationManager *manager;

	guint added_handler_id;
	guint removed_handler_id;

	GtkBuilder *builder;
	GtkUIManager *uimanager;
	GtkWidget *panel_widget;

	guint ui_id;
	GtkActionGroup *action_group;

	guint active_tab_changed_handler_id;
	GtkWidget *scrolled_window_user_view;
	GtkWidget *tree_view_user_view;
};

G_END_DECLS

#endif /* __GEDIT_COLLABORATION_WINDOW_HELPER_PRIVATE_H__ */

