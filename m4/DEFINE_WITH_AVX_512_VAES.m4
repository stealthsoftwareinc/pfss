dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_AVX_512_VAES], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use AVX-512 VAES],
  [WITH_AVX_512_VAES],
  [avx-512-vaes],
  [yes],
  [permit the use of AVX-512 VAES],
  [forbid the use of AVX-512 VAES])[

:;}]])
