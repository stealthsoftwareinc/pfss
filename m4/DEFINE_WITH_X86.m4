dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_X86], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_X86_32])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_X86_64])[

  ]GATBPS_CHECK_EXPR(
    [whether an x86 instruction set is being used],
    [WITH_X86],
    [0
      || ]GATBPS_SOFT_VAR([WITH_X86_32], [0])[
      || ]GATBPS_SOFT_VAR([WITH_X86_64], [0])[
    ])[

}]])
