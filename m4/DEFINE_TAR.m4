dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_TAR], [[{

#
# The block that contains this comment is the expansion of the
# DEFINE_TAR macro.
#]dnl
m4_ifdef(
  [DEFINE_TAR_HAS_BEEN_CALLED],
  [gatbps_fatal([
    DEFINE_TAR has already been called
  ])],
  [m4_define([DEFINE_TAR_HAS_BEEN_CALLED])])[]dnl
m4_if(
  m4_eval([$# != 0]),
  [1],
  [gatbps_fatal([
    DEFINE_TAR requires exactly 0 arguments
    ($# ]m4_if([$#], [1], [[was]], [[were]])[ given)
  ])])[]dnl
[

]AC_REQUIRE([GATBPS_PROG_TAR])[

:;}]])[]dnl
