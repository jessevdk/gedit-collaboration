# <?= $plugin->title ?> plugin
plugindir = $(GEDIT_PLUGINS_LIBS_DIR)

INCLUDES = \
	-I$(top_srcdir) 				\
	$(GEDIT_CFLAGS) 				\
	$(WARN_CFLAGS)

plugin_LTLIBRARIES = libcollaboration.la

libcollaboration_la_SOURCES = \
	gedit-collaboration-plugin.h				\
	gedit-collaboration-plugin.c				\
	gedit-collaboration-window-helper.h			\
	gedit-collaboration-window-helper.c

libcollaboration_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libcollaboration_la_LIBADD = $(GEDIT_LIBS)

# Plugin Info

plugin_in_files = collaboration.gedit-plugin.desktop.in

%.gedit-plugin: %.gedit-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.gedit-plugin.desktop.in=.gedit-plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

-include $(top_srcdir)/git.mk
