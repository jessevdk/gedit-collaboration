/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * gedit-collaboration-plugin.c
 * This file is part of gedit-collaboration-plugin
 *
 * Copyright (C) 2010  Jesse van den Kieboom
 *
 * gedit-collaboration-plugin is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit-collaboration-plugin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit-collaboration-plugin; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "gedit-collaboration-plugin.h"
#include "gedit-collaboration-window-helper.h"
#include "gedit-collaboration-bookmarks.h"
#include "gedit-collaboration-bookmark.h"
#include "gedit-collaboration-bookmark-dialog.h"
#include "gedit-collaboration-manager.h"
#include "gedit-collaboration-color-button.h"
#include "gedit-collaboration-document-message.h"

#include <libinfinity/common/inf-init.h>

#define GEDIT_COLLABORATION_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPluginPrivate))

#define WINDOW_DATA_KEY "GeditCollaborationPluginWindowData"

struct _GeditCollaborationPluginPrivate
{
	GtkWidget *dialog_configuration;
	GtkEntry *entry_name;
	GeditCollaborationColorButton *color_button_hue;
};

GEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE (GeditCollaborationPlugin, gedit_collaboration_plugin, \
	gedit_collaboration_window_helper_register_type (type_module); \
	gedit_collaboration_manager_register_type (type_module); \
	gedit_collaboration_bookmark_register_type (type_module); \
	gedit_collaboration_bookmarks_register_type (type_module); \
	gedit_collaboration_bookmark_dialog_register_type (type_module); \
	gedit_collaboration_color_button_register_type (type_module); \
	gedit_collaboration_document_message_register_type (type_module); \
)

static void
gedit_collaboration_plugin_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_plugin_parent_class)->finalize (object);
}

static void
plugin_activate_impl (GeditPlugin *plugin,
                      GeditWindow *window)
{
	GeditCollaborationWindowHelper *helper;

	helper = gedit_collaboration_window_helper_new (window,
	                                                gedit_plugin_get_data_dir (plugin));

	g_object_set_data_full (G_OBJECT (window),
	                        WINDOW_DATA_KEY,
	                        helper,
	                        (GDestroyNotify)g_object_unref);
}

static void
plugin_deactivate_impl (GeditPlugin *plugin,
                        GeditWindow *window)
{
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
gedit_collaboration_plugin_constructed (GObject *object)
{
	gchar *filename;

	filename = g_build_filename (g_get_home_dir (),
	                             ".gnome2",
	                             "gedit",
	                             "collaboration",
	                             "bookmarks.xml",
	                             NULL);

	gedit_collaboration_bookmarks_initialize (filename);
}

static void
on_dialog_configuration_response (GtkWidget                *widget,
                                  gint                      responseid,
                                  GeditCollaborationPlugin *plugin)
{
	GeditCollaborationUser *user;
	const gchar *name;

	user = gedit_collaboration_user_get_default ();
	name = gtk_entry_get_text (plugin->priv->entry_name);

	if (*name)
	{
		gedit_collaboration_user_set_name (user, name);
	}

	gedit_collaboration_user_set_hue (user,
	                                  gedit_collaboration_color_button_get_hue (plugin->priv->color_button_hue));

	gtk_widget_destroy (widget);
	plugin->priv->dialog_configuration = NULL;
}

static void
create_configuration_dialog (GeditCollaborationPlugin *plugin)
{
	GtkBuilder *builder = gtk_builder_new ();
	GError *error = NULL;
	gchar *filename;

	filename = g_build_filename (gedit_plugin_get_data_dir (GEDIT_PLUGIN (plugin)),
	                             "gedit-collaboration-configuration.ui",
	                             NULL);

	if (!gtk_builder_add_from_file (builder, filename, &error))
	{
		g_warning ("Could not load collaboration configuration dialog from file: %s", filename);

		g_free (filename);
		g_object_unref (builder);

		g_error_free (error);
		return;
	}

	plugin->priv->dialog_configuration = GTK_WIDGET (gtk_builder_get_object (builder, "dialog_configuration"));
	plugin->priv->entry_name = GTK_ENTRY (gtk_builder_get_object (builder, "entry_name"));
	plugin->priv->color_button_hue = GEDIT_COLLABORATION_COLOR_BUTTON (gtk_builder_get_object (builder, "color_button_hue"));
}

static GtkWidget *
plugin_create_configure_dialog_impl (GeditPlugin *plugin)
{
	GeditCollaborationPlugin *self = GEDIT_COLLABORATION_PLUGIN (plugin);
	GeditCollaborationUser *user = gedit_collaboration_user_get_default ();

	if (self->priv->dialog_configuration == NULL)
	{
		create_configuration_dialog (self);

		if (!self->priv->dialog_configuration)
		{
			return NULL;
		}

		g_signal_connect (self->priv->dialog_configuration,
		                  "response",
		                  G_CALLBACK (on_dialog_configuration_response),
		                  self);
	}

	gtk_entry_set_text (self->priv->entry_name,
	                    gedit_collaboration_user_get_name (user));

	gedit_collaboration_color_button_set_hue (self->priv->color_button_hue,
	                                          gedit_collaboration_user_get_hue (user));

	return self->priv->dialog_configuration;
}

static void
plugin_update_ui_impl (GeditPlugin *plugin,
                       GeditWindow *window)
{
	GeditCollaborationWindowHelper *helper;

	helper = g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY);

	if (helper)
	{
		gedit_collaboration_window_helper_update_ui (helper);
	}
}

static void
gedit_collaboration_plugin_class_init (GeditCollaborationPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = gedit_collaboration_plugin_finalize;
	object_class->constructed = gedit_collaboration_plugin_constructed;

	plugin_class->activate = plugin_activate_impl;
	plugin_class->deactivate = plugin_deactivate_impl;
	plugin_class->create_configure_dialog = plugin_create_configure_dialog_impl;
	plugin_class->update_ui = plugin_update_ui_impl;

	g_type_class_add_private (object_class, sizeof(GeditCollaborationPluginPrivate));
}

static void
gedit_collaboration_plugin_init (GeditCollaborationPlugin *plugin)
{
	plugin->priv = GEDIT_COLLABORATION_PLUGIN_GET_PRIVATE (plugin);

	inf_init (NULL);
}
