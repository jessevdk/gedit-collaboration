# -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-

# collaboration plugin
plugindir = $(GEDIT_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 						\
	$(GEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	-DGEDIT_PLUGINS_LIBS_DIR="\"$(GEDIT_PLUGINS_LIBS_DIR)\"" 		\
	-DGEDIT_PLUGINS_DATA_DIR="\"$(GEDIT_PLUGINS_DATA_DIR)\""

plugin_LTLIBRARIES = libcollaboration.la

libcollaboration_la_SOURCES = \
	gedit-collaboration-plugin.h				\
	gedit-collaboration-plugin.c				\
	gedit-collaboration.h					\
	gedit-collaboration.c					\
	gedit-collaboration-window-helper.h			\
	gedit-collaboration-window-helper-private.h		\
	gedit-collaboration-window-helper.c			\
	gedit-collaboration-manager.h				\
	gedit-collaboration-manager.c				\
	gedit-collaboration-actions.h				\
	gedit-collaboration-actions.c				\
	gedit-collaboration-bookmarks.h				\
	gedit-collaboration-bookmarks.c				\
	gedit-collaboration-bookmark.h				\
	gedit-collaboration-bookmark.c				\
	gedit-collaboration-bookmark-dialog.h			\
	gedit-collaboration-bookmark-dialog.c			\
	gedit-collaboration-user.h				\
	gedit-collaboration-user.c				\
	gedit-collaboration-color-button.h			\
	gedit-collaboration-color-button.c			\
	gedit-collaboration-document-message.h			\
	gedit-collaboration-document-message.c			\
	gedit-collaboration-hue-renderer.h			\
	gedit-collaboration-hue-renderer.c			\
	gedit-collaboration-user-store.h			\
	gedit-collaboration-user-store.c			\
	gedit-collaboration-undo-manager.h			\
	gedit-collaboration-undo-manager.c

libcollaboration_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libcollaboration_la_LIBADD = $(GEDIT_LIBS)

# Plugin Info
plugin_in_files = collaboration.gedit-plugin.desktop.in

# UI files (if you use ui for your plugin, list those files here)
uidir = $(GEDIT_PLUGINS_DATA_DIR)/collaboration
ui_DATA = \
	gedit-collaboration-window-helper.ui \
	gedit-collaboration-bookmark-dialog.ui \
	gedit-collaboration-configuration.ui \
	gedit-collaboration-password-dialog.ui

%.gedit-plugin: %.gedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.gedit-plugin.desktop.in=.gedit-plugin)

EXTRA_DIST = \
	$(plugin_in_files) \
	$(ui_DATA)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
