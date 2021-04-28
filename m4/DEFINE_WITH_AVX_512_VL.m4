dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_AVX_512_VL], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use AVX-512 VL],
  [WITH_AVX_512_VL],
  [avx-512-vl],
  [yes],
  [permit the use of AVX-512 VL],
  [forbid the use of AVX-512 VL])[

:;}]])
