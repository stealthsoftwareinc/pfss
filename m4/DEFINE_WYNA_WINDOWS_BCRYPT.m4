dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WYNA_WINDOWS_BCRYPT], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_WYNAS])

  GATBPS_ARG_WYNA(
    [--with-windows-bcrypt],
    [
      Use the Windows bcrypt API.
    ],
    [
      Do not use the Windows bcrypt API.
    ],
    [
      Automatically decide whether to use the Windows bcrypt API.
    ],
    [
      --with-windows
    ])

}])
