dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_ARM_CRYPTO], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use ARM Crypto],
  [WITH_ARM_CRYPTO],
  [arm-crypto],
  [yes],
  [permit the use of ARM Crypto],
  [forbid the use of ARM Crypto])[

:;}]])
