dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl


AC_INIT(
  [[PFSS]],
  m4_esyscmd_s([
    sh build-aux/VERSION.sh
  ])m4_if(m4_sysval, [0], [], [m4_fatal([VERSION.sh failed])]),
  [[pfss@stealthsoftwareinc.com]],
  [[pfss]])

m4_include([build-aux/gatbps.ac])

[

]GATBPS_DEFINE_VERSION(
  [AC_],
  [PACKAGE_VERSION])[

]GATBPS_DEFINE_DATE(
  [AC_],
  [PACKAGE_DATE],
  m4_esyscmd_s([
    sh build-aux/DATE.sh
  ])m4_if(m4_sysval, [0], [], [m4_fatal([DATE.sh failed])]))[

]AC_CONFIG_AUX_DIR([build-aux])[
]AC_CONFIG_MACRO_DIR([m4])[
]AC_CONFIG_MACRO_DIRS([m4])[
]AC_CONFIG_SRCDIR([src/c_cpp/include/pfss.h])[

]AM_INIT_AUTOMAKE([
  -Wall
  foreign
  nostdinc
  subdir-objects
  tar-ustar
])[

]m4_include([build-aux/autogen.ac])[

]LT_INIT([
  win32-dll
])[

]dnl begin_includes
[

]dnl end_includes
[

]GATBPS_ARG_WITH_BOOL(
  [--with-logging],
  [WITH_LOGGING],
  [logging],
  [no],
  [enable logging features],
  [disable logging features])[

]dnl begin_prerequisites
[

]GATBPS_CONFIG_FILE_SUBST[
]GATBPS_CONFIG_LATER_SUBST[
]GATBPS_CP_SUBST[
]GATBPS_DOCKER_BUILD_SUBST[
]GATBPS_GIT_ARCHIVE_SUBST[
]GATBPS_GIT_BUNDLE_SUBST[
]GATBPS_GIT_CLONE_SUBST[
]GATBPS_WGET_SUBST[

]dnl end_prerequisites
[

]AC_CONFIG_HEADERS([src/c_cpp/include/config.h])[
]GATBPS_CONFIG_FILE([Makefile])[
]GATBPS_CONFIG_FILE([src/c_cpp/include/pfss/config.h])[

]AC_OUTPUT[

]
