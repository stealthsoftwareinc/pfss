dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN([DEFINE_LIB_CXXFLAGS], [[{

#
# The block that contains this comment is the expansion of the
# DEFINE_LIB_CXXFLAGS macro.
#

]m4_ifdef(
  [DEFINE_LIB_CXXFLAGS_HAS_BEEN_CALLED],
  [gatbps_fatal([
    DEFINE_LIB_CXXFLAGS has already been called
  ])],
  [m4_define([DEFINE_LIB_CXXFLAGS_HAS_BEEN_CALLED])])[

]m4_if(
  m4_eval([$# != 0]),
  [1],
  [gatbps_fatal([
    DEFINE_LIB_CXXFLAGS requires exactly 0 arguments
    ($# ]m4_if([$#], [1], [[was]], [[were]])[ given)
  ])])[

]AC_REQUIRE([GATBPS_DEFINE_LIB_CXXFLAGS])[

:;}]])[]dnl
