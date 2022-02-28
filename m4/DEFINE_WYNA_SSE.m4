dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WYNA_SSE], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_WYNAS])

  GATBPS_ARG_WYNA(
    [--with-sse],
    [
      Use the SSE instruction set.
    ],
    [
      Do not use the SSE instruction set.
    ],
    [
      Automatically decide whether to use the SSE instruction set.
    ],
    [
    ])

}])
