dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT(gedit-collaboration, 0.1)

AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_PROGRAM([gedit])

AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip])

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS([config.h])
AC_DISABLE_STATIC

IT_PROG_INTLTOOL([0.35.0])
AC_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl check for win32 platform
AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
	*-*-mingw*|*-*-cygwin*)
		platform_win32=yes
		;;
	*)
		platform_win32=no
		;;
esac

AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

GETTEXT_PACKAGE=geditcollaboration
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
AM_GLIB_GNU_GETTEXT

dnl ================================================================
dnl Start of pkg-config checks
dnl ================================================================
GEDIT_REQUIRED_VERSION=2.90
INFINITY_REQUIRED_VERSION=0.4.1

PKG_CHECK_MODULES(GEDIT, [
	gedit >= $GEDIT_REQUIRED_VERSION,
	gtk+-3.0 >= 2.90.0
	libinfinity-0.5 >= $INFINITY_REQUIRED_VERSION
	libinfgtk-0.5 >= $INFINITY_REQUIRED_VERSION
	libinftextgtk-0.5 >= $INFINITY_REQUIRED_VERSION
	libxml-2.0
])

if test "$USE_MAINTAINER_MODE" = "yes"; then
	GEDIT_CFLAGS="$GEDIT_CFLAGS -Wall -Werror"
fi

dnl ================================================================
dnl GSettings stuff
dnl ================================================================

GLIB_GSETTINGS

dnl ================================================================
dnl Misc
dnl ================================================================
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

if test "$platform_win32" = yes; then
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version -no-undefined"
else
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
fi
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

GEDIT_PLUGINS_LIBS_DIR="$libdir/gedit/plugins"
AC_SUBST(GEDIT_PLUGINS_LIBS_DIR)

GEDIT_PLUGINS_DATA_DIR="$datadir/gedit/plugins"
AC_SUBST(GEDIT_PLUGINS_DATA_DIR)

AC_CONFIG_FILES([
Makefile
data/Makefile
src/Makefile
src/collaboration.plugin.desktop.in
po/Makefile.in
data/org.gnome.gedit.plugins.collaboration.gschema.xml.in
])

AC_OUTPUT

echo "

Configuration:

	Source code location:   ${srcdir}
	Compiler:               ${CC}
	Prefix:			${prefix}
	Stable:                 ${geditdev}

Note: you have to install this plugin into the same prefix as your gedit
installation (probably /usr if you're using your distro packages, /usr/local
if you have compiled it on your own).
"
