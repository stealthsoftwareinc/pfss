dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_SSE2], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use SSE2],
  [WITH_SSE2],
  [sse2],
  [yes],
  [permit the use of SSE2],
  [forbid the use of SSE2])[

:;}]])
