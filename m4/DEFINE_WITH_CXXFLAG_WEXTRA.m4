dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_CXXFLAG_WEXTRA], [[{

]GATBPS_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -Wextra],
  [WITH_CXXFLAG_WEXTRA],
  [-Wextra])[

case $WITH_CXXFLAG_WEXTRA in
  1)
    CXXFLAGS="$CXXFLAGS -Wextra"
  ;;
esac

:;}]])
