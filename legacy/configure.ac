
dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([reinstall], [6.1.0], [perry.werneck@gmail.com],[reinstall],[https://github.com/PerryWerneck/reinstall])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

dnl Set gettext version
AM_GNU_GETTEXT_VERSION([0.14])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S

AC_LANG([C++])

AC_PATH_TOOL([STRIP], [strip])
AC_SUBST(STRIP)
AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

AC_SUBST(PACKAGE_DESCRIPTION,"Instalar/Reinstalar terminal")

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)
app_vrs_release=$(echo $VERSION | cut -d. -f3)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major, [The library major version])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor, [The library minor version])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_release, [The library release number])

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_ARG_WITH([release], [AS_HELP_STRING([--with-release], [Set release])], [ app_cv_release="$withval" ],[ app_cv_release="`date +%-y`.`date +%-m`.`date +%-d`" ])

app_rls_major=$(echo $app_cv_release.0.0 | cut -d. -f1)
app_rls_minor=$(echo $app_cv_release.0.0 | cut -d. -f2)

app_vrs_revision=$(date +%Y%m%d)
AC_SUBST(PACKAGE_REVISION,$app_vrs_revision)
AC_DEFINE_UNQUOTED(PACKAGE_REVISION,$app_vrs_revision,[The package revision (for win32 installer)])

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_RELEASE, $app_rls_major, [The library major release])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_RELEASE, $app_rls_minor, [The library minor release])
AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, "$app_cv_release", [The package release])

AC_SUBST(PACKAGE_RELEASE,"$app_cv_release")
AC_SUBST(PACKAGE_MAJOR_RELEASE,$app_rls_major)
AC_SUBST(PACKAGE_MINOR_RELEASE,$app_rls_minor)

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

CFLAGS="$CFLAGS -pthread"
LDFLAGS="$LDFLAGS -pthread"
DLL_LDFLAGS="-shared -Wl,-soname,\$(@F)"
AC_SUBST(SONAME,libreinstall.so.$app_vrs_major.$app_vrs_minor)

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------
AC_PATH_TOOL([DOXYGEN], [doxygen])
AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([INSTALL_DESKTOP],[desktop-file-install],[no])

dnl ---------------------------------------------------------------------------
dnl Check for C++ 2017 support
dnl ---------------------------------------------------------------------------
AC_DEFUN([AX_CHECK_COMPILE_FLAG],
[AC_PREREQ(2.59)dnl for _AC_LANG_PREFIX
AS_VAR_PUSHDEF([CACHEVAR],[ax_cv_check_[]_AC_LANG_ABBREV[]flags_$4_$1])dnl
AC_CACHE_CHECK([whether _AC_LANG compiler accepts $1], CACHEVAR, [
  ax_check_save_flags=$[]_AC_LANG_PREFIX[]FLAGS
  _AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $4 $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
    [AS_VAR_SET(CACHEVAR,[yes])],
    [AS_VAR_SET(CACHEVAR,[no])])
  _AC_LANG_PREFIX[]FLAGS=$ax_check_save_flags])
AS_IF([test x"AS_VAR_GET(CACHEVAR)" = xyes],
  [m4_default([$2], :)],
  [m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])dnl AX_CHECK_COMPILE_FLAGS

AX_CHECK_COMPILE_FLAG([-std=c++1z], [CPPFLAGS="$CPPFLAGS -std=c++1z"], [app_cv_c17=no])

if test "$app_cv_c17" == "no"; then
	AX_CHECK_COMPILE_FLAG([-std=c++11], [CPPFLAGS="$CPPFLAGS -std=c++11 -DLIBUDJAT_LEGACY=1"], [app_cv_c11=no])
fi

dnl ---------------------------------------------------------------------------
dnl Check for libudjat
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [UDJAT], [libudjat], AC_DEFINE(HAVE_UDJAT,,[Do we have udjat?]), AC_MSG_ERROR([UDJAT not present.]) )
AC_SUBST(UDJAT_LIBS)
AC_SUBST(UDJAT_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libudjatdbus
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [DBUS], [libudjatdbus], AC_DEFINE(HAVE_UDJAT_DBUS,,[Do we have udjat dbus?]), AC_MSG_ERROR([libudjatdbus not present.]) )
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libudjatdbus
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [HTTP], [libudjathttp], AC_DEFINE(HAVE_UDJAT_HTTP,,[Do we have udjat http?]), AC_MSG_ERROR([libudjathttp not present.]) )
AC_SUBST(HTTP_LIBS)
AC_SUBST(HTTP_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libslp
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(slp.h, [app_cv_libslp="yes"], [AC_MSG_NOTICE([Can't find required file slp.h])])

if test x$app_cv_libslp = xyes; then
        AC_DEFINE(HAVE_LIBSLP, 1, [OpenSLP is available])
        AC_SUBST(LIBSLP_LIBS,"-lslp")
else
        AC_SUBST(LIBSLP_LIBS,"")
fi

AC_SUBST(LIBSLP_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libisofs
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [ISOFS], [libisofs-1], AC_DEFINE(HAVE_LIBISOFS,,[ Do we have isofs? ]), AC_MSG_NOTICE([isofs not present.]))

AC_SUBST(ISOFS_LIBS)
AC_SUBST(ISOFS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for fatfs
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [FATFS], [libfatfs], AC_DEFINE(HAVE_FATFS,,[ Do we have fatfs? ]), AC_MSG_ERROR([fatfs not present.]))

AC_SUBST(FATFS_LIBS)
AC_SUBST(FATFS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for zlib
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [ZLIB], [zlib], AC_DEFINE(HAVE_ZLIB,,[ "Do we have zlib" ]), AC_MSG_ERROR([zlib not present.]))

AC_SUBST(ZLIB_LIBS)
AC_SUBST(ZLIB_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(sysconf/tmpfiles.d)
AC_CONFIG_FILES(sysconf/launcher.desktop)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT

