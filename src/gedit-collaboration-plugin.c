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

#include <gedit/gedit-app-activatable.h>
#include <libpeas-gtk/peas-gtk-configurable.h>
#include <gedit/gedit-app.h>

#include "gedit-collaboration-plugin.h"
#include "gedit-collaboration-window-helper.h"
#include "gedit-collaboration-bookmarks.h"
#include "gedit-collaboration-bookmark.h"
#include "gedit-collaboration-bookmark-dialog.h"
#include "gedit-collaboration-manager.h"
#include "gedit-collaboration-color-button.h"
#include "gedit-collaboration-document-message.h"
#include "gedit-collaboration.h"
#include "gedit-collaboration-undo-manager.h"
#include "gedit-collaboration-user-store.h"
#include "gedit-collaboration-hue-renderer.h"

#include <libinfinity/common/inf-init.h>

#define GEDIT_COLLABORATION_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPluginPrivate))

#define WINDOW_DATA_KEY "GeditCollaborationPluginWindowData"

static GeditCollaborationPlugin *plugin_instance = 0;

struct _GeditCollaborationPluginPrivate
{
	GeditApp *app;

	GtkWidget *dialog_configuration;
	GtkEntry *entry_name;
	GeditCollaborationColorButton *color_button_hue;
};

/* Properties */
enum
{
	PROP_0,
	PROP_APP
};

static void gedit_app_activatable_iface_init (GeditAppActivatableInterface *iface);
static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditCollaborationPlugin,
                                gedit_collaboration_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_APP_ACTIVATABLE,
                                                               gedit_app_activatable_iface_init) \
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init) \
                                _gedit_collaboration_window_helper_register_type (type_module); \
                                _gedit_collaboration_manager_register_type (type_module); \
                                _gedit_collaboration_bookmark_register_type (type_module); \
                                _gedit_collaboration_bookmarks_register_type (type_module); \
                                _gedit_collaboration_bookmark_dialog_register_type (type_module); \
                                _gedit_collaboration_color_button_register_type (type_module); \
                                _gedit_collaboration_document_message_register_type (type_module); \
                                _gedit_collaboration_undo_manager_register_type (type_module); \
                                _gedit_collaboration_user_store_register_type (type_module); \
                                _gedit_collaboration_hue_renderer_register_type (type_module); \
)

static void
gedit_app_activatable_iface_init (GeditAppActivatableInterface *iface)
{
}

static void
on_entry_name_focus_out (GtkEntry               *entry,
                         GdkEventFocus          *event,
                         GeditCollaborationUser *user)
{
	const gchar *name = gtk_entry_get_text (entry);
	gedit_collaboration_user_set_name (user, name);
}

static void
on_color_button_hue_changed (GeditCollaborationColorButton *button,
                             GParamSpec                    *spec,
                             GeditCollaborationUser        *user)
{
	gedit_collaboration_user_set_hue (user,
	                                  gedit_collaboration_color_button_get_hue (button));
}

static GtkWidget *
gedit_collaboration_plugin_create_configure_widget (PeasGtkConfigurable *configurable)
{
	GtkBuilder *builder;
	gchar *datadir;
	GtkEntry *entry;
	GeditCollaborationColorButton *color_button;
	GeditCollaborationUser *user;
	GtkWidget *ret;

	datadir = peas_extension_base_get_data_dir (PEAS_EXTENSION_BASE (configurable));
	builder = gedit_collaboration_create_builder (datadir,
	                                              "gedit-collaboration-configuration.ui");
	g_free (datadir);

	if (!builder)
	{
		return NULL;
	}

	user = gedit_collaboration_user_get_default ();

	entry = GTK_ENTRY (gtk_builder_get_object (builder, "entry_name"));
	g_signal_connect (entry,
	                  "focus-out-event",
	                  G_CALLBACK (on_entry_name_focus_out),
	                  user);

	color_button = GEDIT_COLLABORATION_COLOR_BUTTON (gtk_builder_get_object (builder, "color_button_hue"));
	g_signal_connect (color_button,
	                  "notify::hue",
	                  G_CALLBACK (on_color_button_hue_changed),
	                  user);

	gtk_entry_set_text (entry,
	                    gedit_collaboration_user_get_name (user));

	gedit_collaboration_color_button_set_hue (color_button,
	                                          gedit_collaboration_user_get_hue (user));

	ret = g_object_ref (gtk_builder_get_object (builder, "vbox_configuration"));
	g_object_unref (builder);

	return ret;
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = gedit_collaboration_plugin_create_configure_widget;
}

static void
gedit_collaboration_plugin_finalize (GObject *object)
{
	G_OBJECT_CLASS (gedit_collaboration_plugin_parent_class)->finalize (object);
}

static void
gedit_collaboration_plugin_constructed (GObject *object)
{
	gchar *filename;

	filename = g_build_filename (g_get_user_config_dir (),
	                             "gedit",
	                             "plugins",
	                             "collaboration",
	                             "bookmarks.xml",
	                             NULL);

	gedit_collaboration_bookmarks_initialize (filename);
}

static GObject *
gedit_collaboration_plugin_constructor (GType                  type,
                                        guint                  n_parameters,
                                        GObjectConstructParam *parameters)
{
	GObject *ret;

	if (plugin_instance != NULL)
	{
		return g_object_ref (plugin_instance);
	}

	ret = G_OBJECT_CLASS (gedit_collaboration_plugin_parent_class)->constructor (type,
	                                                                             n_parameters,
	                                                                             parameters);

	g_object_add_weak_pointer (ret,
	                           (gpointer *)&ret);
	return ret;
}

static void
gedit_collaboration_plugin_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
	GeditCollaborationPlugin *self = GEDIT_COLLABORATION_PLUGIN (object);

	switch (prop_id)
	{
		case PROP_APP:
			self->priv->app = g_value_get_object (value);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_plugin_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
	GeditCollaborationPlugin *self = GEDIT_COLLABORATION_PLUGIN (object);

	switch (prop_id)
	{
		case PROP_APP:
			g_value_set_object (value, self->priv->app);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gedit_collaboration_plugin_class_init (GeditCollaborationPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gedit_collaboration_plugin_finalize;

	object_class->get_property = gedit_collaboration_plugin_get_property;
	object_class->set_property = gedit_collaboration_plugin_set_property;

	object_class->constructed = gedit_collaboration_plugin_constructed;
	object_class->constructor = gedit_collaboration_plugin_constructor;

	g_type_class_add_private (object_class, sizeof(GeditCollaborationPluginPrivate));

	g_object_class_install_property (object_class,
	                                 PROP_APP,
	                                 g_param_spec_object ("app",
	                                                      "App",
	                                                      "App",
	                                                      GEDIT_TYPE_APP,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
gedit_collaboration_plugin_class_finalize (GeditCollaborationPluginClass *klass)
{
}

static void
gedit_collaboration_plugin_init (GeditCollaborationPlugin *plugin)
{
	plugin->priv = GEDIT_COLLABORATION_PLUGIN_GET_PRIVATE (plugin);

	inf_init (NULL);
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	gedit_collaboration_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            GEDIT_TYPE_APP_ACTIVATABLE,
	                                            GEDIT_TYPE_COLLABORATION_PLUGIN);

	peas_object_module_register_extension_type (module,
	                                            PEAS_GTK_TYPE_CONFIGURABLE,
	                                            GEDIT_TYPE_COLLABORATION_PLUGIN);
}
