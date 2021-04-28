dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_AES_NI], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use AES-NI],
  [WITH_AES_NI],
  [aes-ni],
  [yes],
  [permit the use of AES-NI],
  [forbid the use of AES-NI])[

:;}]])
