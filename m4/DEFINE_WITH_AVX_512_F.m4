dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_AVX_512_F], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use AVX-512 F],
  [WITH_AVX_512_F],
  [avx-512-f],
  [yes],
  [permit the use of AVX-512 F],
  [forbid the use of AVX-512 F])[

:;}]])
