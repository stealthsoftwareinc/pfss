dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WYNA_WINDOWS_KERNEL32], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_WYNAS])

  GATBPS_ARG_WYNA(
    [--with-windows-kernel32],
    [
      Use the Windows kernel32 API.
    ],
    [
      Do not use the Windows kernel32 API.
    ],
    [
      Automatically decide whether to use the Windows kernel32 API.
    ],
    [
      --with-windows
    ])

}])
