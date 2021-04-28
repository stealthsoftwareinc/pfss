dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_NETTLE], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use Nettle's -lnettle],
  [WITH_NETTLE],
  [nettle],
  [yes],
  [permit the use of Nettle's -lnettle],
  [forbid the use of Nettle's -lnettle])[

:;}]])
