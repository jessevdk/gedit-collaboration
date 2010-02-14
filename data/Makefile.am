iconsdir = $(GEDIT_PLUGINS_DATA_DIR)/collaboration/icons
icons_DATA = people.svg

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = gedit-collaboration.schemas.in
schemas_DATA  = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	        if test -z "$(DESTDIR)" ; then \
	                for p in $(schemas_DATA) ; do \
	                        GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p ; \
	                done \
	        fi
else
install-data-local:
endif

EXTRA_DIST = \
	$(icons_DATA) \
	$(schemas_in_files)

CLEANFILES = \
	$(schemas_DATA)

-include $(top_srcdir)/git.mk
