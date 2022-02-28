dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_CXXFLAG_WALL], [[{

]GATBPS_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -Wall],
  [WITH_CXXFLAG_WALL],
  [-Wall])[

case $WITH_CXXFLAG_WALL in
  1)
    CXXFLAGS="$CXXFLAGS -Wall"
  ;;
esac

:;}]])
