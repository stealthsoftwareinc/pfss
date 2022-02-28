dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_AES_NI], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])[
  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_SSE2])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_SSE])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]GATBPS_FINISH_WYNA(
    [--with-aes-ni],
    [WYNA_AES_NI])[

  ]m4_pushdef(
    [prologue],
    [[
      #include <emmintrin.h>
      #include <wmmintrin.h>
    ]])[

  ]m4_pushdef(
    [body],
    [[
      (void)_mm_aesimc_si128(_mm_setzero_si128());
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_LINK(
    [for the AES-NI instruction set (C link)],
    [WITH_AES_NI_C_LINK],
    prologue,
    body,
    [1
      && !WYNA_AES_NI_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
      && ]GATBPS_SOFT_VAR([WITH_SSE])[
      && ]GATBPS_SOFT_VAR([WITH_SSE2])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for the AES-NI instruction set (C run)],
    [WITH_AES_NI_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_AES_NI_C_LINK])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_LINK(
    [for the AES-NI instruction set (C++ link)],
    [WITH_AES_NI_CPP_LINK],
    prologue,
    body,
    [1
      && !WYNA_AES_NI_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      && ]GATBPS_SOFT_VAR([WITH_SSE])[
      && ]GATBPS_SOFT_VAR([WITH_SSE2])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for the AES-NI instruction set (C++ run)],
    [WITH_AES_NI_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_AES_NI_CPP_LINK])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [for the AES-NI instruction set (all)],
    [WITH_AES_NI],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_AES_NI_C_RUN
        || (1
          && WYNA_AES_NI_is_yes
          && WITH_AES_NI_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_AES_NI_CPP_RUN
        || (1
          && WYNA_AES_NI_is_yes
          && WITH_AES_NI_CPP_RUN_is_guess
        )
      )
    ])[

  case $][{WYNA_AES_NI_is_yes?}$][{WITH_AES_NI?} in 10)
    ]GATBPS_BARF([
      The AES-NI instruction set is not available.
    ])[
  esac

}]])
