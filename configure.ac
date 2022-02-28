dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

dnl---------------------------------------------------------------------

AC_INIT(
  [[PFSS]],
  m4_esyscmd_s([
    sh build-aux/gatbps-gen-version.sh
  ])m4_if(m4_sysval, 0, [], [
    m4_fatal([gatbps-gen-version.sh failed.])
  ]),
  [[pfss@stealthsoftwareinc.com]],
  [[pfss]])

dnl---------------------------------------------------------------------

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([build-aux/pfss.srcdir])

AM_INIT_AUTOMAKE([
  -Wall
  foreign
  nostdinc
  subdir-objects
  tar-ustar
])

dnl---------------------------------------------------------------------

m4_include([build-aux/gatbps.ac])

dnl---------------------------------------------------------------------
dnl Program checks
dnl---------------------------------------------------------------------

GATBPS_PROG_ASCIIDOCTOR
GATBPS_PROG_AWK
GATBPS_PROG_CURL
GATBPS_PROG_DOCKER
GATBPS_PROG_GIT
GATBPS_PROG_GREP
GATBPS_PROG_GUNZIP
GATBPS_PROG_LN_S
GATBPS_PROG_MKDIR_P
GATBPS_PROG_OPENSSL
GATBPS_PROG_SED
GATBPS_PROG_TAR
GATBPS_PROG_WGET
GATBPS_PROG_XZ
GATBPS_PROG_ZIP

dnl---------------------------------------------------------------------

[

]GATBPS_DEFINE_VERSION(
  [AC_],
  [PACKAGE_VERSION])[

]GATBPS_DEFINE_DATE(
  [AC_],
  [PACKAGE_DATE],
  m4_esyscmd_s([
    sh build-aux/gatbps-gen-date.sh
  ])m4_if(m4_sysval, 0, [], [
    m4_fatal([gatbps-gen-date.sh failed])
  ]))[

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
