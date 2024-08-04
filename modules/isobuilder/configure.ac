
dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([isobuilder], [5.1.0], [perry.werneck@gmail.com],[isobuilder],[https://github.com/PerryWerneck/reinstall])

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

AC_SUBST(PACKAGE_DESCRIPTION,"ISO Image writer")

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(unistd.h)

case "$host" in
	*-mingw32|*-pc-msys)
		EXEEXT=".exe"
		OSNAME="windows"

		CPPFLAGS="$CPPFLAGS -D_WIN32_WINNT=0x0600"
		LIBEXT=".dll"

		app_win32_revision=$(date +%y.%m.%d.%H)
		AC_SUBST(WIN32_VERSION,$app_win32_revision)

		app_win32_fileversion=$(date +%_y,%_m,%_d,%_H)
		AC_SUBST(WIN32_FILEVERSION,$app_win32_fileversion)

		if test "$host_cpu" = "x86_64"; then
			app_cv_programfiles="PROGRAMFILES64"
			app_cv_winarch="64"
		else
			app_cv_programfiles="PROGRAMFILES"
			app_cv_winarch="32"
		fi
		
		AC_SUBST(PROGRAMFILES,$app_cv_programfiles)
		AC_SUBST(WINARCH,$app_cv_winarch)

		AC_PATH_TOOL([WINDRES], [windres], [no])
		AC_PATH_TOOL([ZIP], [zip], [no])
dnl		AC_CONFIG_FILES(src/os/windows/resources.rc)
dnl		AC_CONFIG_FILES(src/os/windows/app-manifest.xml)
dnl		AC_CONFIG_FILES(win/userinfo.nsi)
		;;

 	*)
		EXEEXT=""
		LIBEXT=".so"
		OSNAME="linux"
		CPPFLAGS="$CPPFLAGS -fPIC -pthread"
		LDFLAGS="$LDFLAGS -pthread"

esac

AC_SUBST(LIBS)
AC_SUBST(EXEEXT)
AC_SUBST(LIBEXT)
AC_SUBST(OSNAME)
AC_SUBST(DLL_LDFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------
AC_PATH_TOOL([VALGRIND], [valgrind], [no])

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

AX_CHECK_COMPILE_FLAG([-std=gnu++17], [CPPFLAGS="$CPPFLAGS -std=gnu++17"])

dnl ---------------------------------------------------------------------------
dnl Check for libudjat
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [UDJAT], [libudjat], AC_DEFINE(HAVE_UDJAT,,[Do we have udjat?]), AC_MSG_ERROR([UDJAT not present.]) )
AC_SUBST(UDJAT_LIBS)
AC_SUBST(UDJAT_CFLAGS)

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product=$PACKAGE ])
AC_DEFINE_UNQUOTED(PRODUCT_NAME, "$app_cv_product",[The product name])
AC_SUBST(PRODUCT_NAME,"$app_cv_product")

AC_ARG_WITH([product-id], [AS_HELP_STRING([--with-product-id], [Set product id])], [ app_cv_product_id="$withval" ],[ app_cv_product_id="`pkg-config --variable=product_id libudjat`" ])
AC_SUBST(PRODUCT_ID,$app_cv_product_id)
AC_DEFINE_UNQUOTED(PRODUCT_ID,"$app_cv_product_id",[The product id])

AC_ARG_WITH([module-path], [AS_HELP_STRING([--with-module-path], [Set module-path])], [ app_cv_module_path="$withval" ],[ app_cv_module_path=`pkg-config --variable=module_path libudjat` ])

AC_DEFINE_UNQUOTED(MODULE_PATH, $app_cv_module_path, [The path for module installation])
AC_SUBST(MODULE_PATH,$app_cv_module_path)

dnl ---------------------------------------------------------------------------
dnl Check for libreinstall
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [REINSTALL], [libreinstall], AC_DEFINE(HAVE_REINSTALL,,[Do we have reinstall?]), AC_MSG_ERROR([libreinstall not present.]) )
AC_SUBST(REINSTALL_LIBS)
AC_SUBST(REINSTALL_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for fatfs
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [FATFS], [libfatfs-static], AC_DEFINE(HAVE_FATFS,,[ Do we have fatfs? ]), AC_MSG_NOTICE([fatfs not present.]))

AC_SUBST(FATFS_LIBS)
AC_SUBST(FATFS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libguestfs
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [GUESTFS], [libguestfs], AC_DEFINE(HAVE_GUESTFS,,[ Do we have libguestfs? ]), AC_MSG_NOTICE([libguestfs not present.]))

AC_SUBST(GUESTFS_LIBS)
AC_SUBST(GUESTFS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for libburn
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [BURN], [libburn-1], AC_DEFINE(HAVE_BURN,[],[Do we have libburn?]), AC_MSG_NOTICE([libburn not present.]) )

AC_SUBST(BURN_LIBS)
AC_SUBST(BURN_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for Libisofs
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [ISOFS], [libisofs-1], AC_DEFINE(HAVE_ISOFS,[],[Do we have libisofs?]), AC_MSG_NOTICE([libisofs not present.]) )

AC_SUBST(ISOFS_LIBS)
AC_SUBST(ISOFS_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT

