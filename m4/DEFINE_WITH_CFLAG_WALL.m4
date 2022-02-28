dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_CFLAG_WALL], [[{

]GATBPS_BEFORE([$0], [DEFINE_CFLAGS])[

]GATBPS_CHECK_CFLAG(
  [CFLAGS += -Wall],
  [WITH_CFLAG_WALL],
  [-Wall])[

case $WITH_CFLAG_WALL in
  1)
    CFLAGS="$CFLAGS -Wall"
  ;;
esac

:;}]])
