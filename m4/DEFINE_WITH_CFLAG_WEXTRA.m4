dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_CFLAG_WEXTRA], [[{

]GATBPS_BEFORE([$0], [DEFINE_CFLAGS])[

]GATBPS_CHECK_CFLAG(
  [CFLAGS += -Wextra],
  [WITH_CFLAG_WEXTRA],
  [-Wextra])[

case $WITH_CFLAG_WEXTRA in
  1)
    CFLAGS="$CFLAGS -Wextra"
  ;;
esac

:;}]])
