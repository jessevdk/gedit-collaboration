iconsdir = $(GEDIT_PLUGINS_DATA_DIR)/collaboration/icons
icons_DATA = people.svg

gsettings_SCHEMAS = org.gnome.gedit.plugins.collaboration.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@

EXTRA_DIST = \
	$(icons_DATA) \
	org.gnome.gedit.plugins.collaboration.gschema.xml.in.in

CLEANFILES = \
	$(gsettings_SCHEMAS)

DISTCLEANFILES = \
	$(gsettings_SCHEMAS)

MAINTAINERCLEANFILES = $(gsettings_SCHEMAS:.xml=.valid)

-include $(top_srcdir)/git.mk
