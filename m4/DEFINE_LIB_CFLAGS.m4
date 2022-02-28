dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_LIB_CFLAGS], [[{

#
# The block that contains this comment is the expansion of the
# DEFINE_LIB_CFLAGS macro.
#

]m4_ifdef(
  [DEFINE_LIB_CFLAGS_HAS_BEEN_CALLED],
  [gatbps_fatal([
    DEFINE_LIB_CFLAGS has already been called
  ])],
  [m4_define([DEFINE_LIB_CFLAGS_HAS_BEEN_CALLED])])[

]m4_if(
  m4_eval([$# != 0]),
  [1],
  [gatbps_fatal([
    DEFINE_LIB_CFLAGS requires exactly 0 arguments
    ($# ]m4_if([$#], [1], [[was]], [[were]])[ given)
  ])])[

]AC_REQUIRE([GATBPS_DEFINE_LIB_CFLAGS])[

:;}]])[]dnl
