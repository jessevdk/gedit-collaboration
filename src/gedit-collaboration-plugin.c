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

/*#define GEDIT_COLLABORATION_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GEDIT_TYPE_COLLABORATION_PLUGIN, GeditCollaborationPluginPrivate))*/

#define WINDOW_DATA_KEY "GeditCollaborationPluginWindowData"

/*struct _GeditCollaborationPluginPrivate
{
};*/

GEDIT_PLUGIN_REGISTER_TYPE_WITH_CODE (GeditCollaborationPlugin, gedit_collaboration_plugin, \
	gedit_collaboration_window_helper_register_type (type_module) \
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
	                        (GDestroyNotify)g_object_unref,
	                        NULL);
}

static void
plugin_deactivate_impl (GeditPlugin *plugin,
                        GeditWindow *window)
{
	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
gedit_collaboration_plugin_class_init (GeditCollaborationPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = gedit_collaboration_plugin_finalize;

	plugin_class->activate = plugin_activate_impl;
	plugin_class->deactivate = plugin_deactivate_impl;

	/*g_type_class_add_private (object_class, sizeof(GeditCollaborationPluginPrivate));*/
}

static void
gedit_collaboration_plugin_init (GeditCollaborationPlugin *plugin)
{
	/*plugin->priv = GEDIT_COLLABORATION_PLUGIN_GET_PRIVATE (plugin);*/
}
