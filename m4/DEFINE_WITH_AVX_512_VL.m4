dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_AVX_512_VL], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])[
  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_AVX_512_F])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_SSE2])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_SSE])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]GATBPS_FINISH_WYNA(
    [--with-avx-512-vl],
    [WYNA_AVX_512_VL])[

  ]m4_pushdef(
    [prologue],
    [[
      #include <immintrin.h>
    ]])[

  ]m4_pushdef(
    [body],
    [[
      (void)_mm256_abs_epi64(_mm256_setzero_si256());
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_LINK(
    [for the AVX-512 VL instruction set (C link)],
    [WITH_AVX_512_VL_C_LINK],
    prologue,
    body,
    [1
      && !WYNA_AVX_512_VL_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
      && ]GATBPS_SOFT_VAR([WITH_SSE])[
      && ]GATBPS_SOFT_VAR([WITH_SSE2])[
      && ]GATBPS_SOFT_VAR([WITH_AVX_512_F])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for the AVX-512 VL instruction set (C run)],
    [WITH_AVX_512_VL_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_AVX_512_VL_C_LINK])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_LINK(
    [for the AVX-512 VL instruction set (C++ link)],
    [WITH_AVX_512_VL_CPP_LINK],
    prologue,
    body,
    [1
      && !WYNA_AVX_512_VL_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      && ]GATBPS_SOFT_VAR([WITH_SSE])[
      && ]GATBPS_SOFT_VAR([WITH_SSE2])[
      && ]GATBPS_SOFT_VAR([WITH_AVX_512_F])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for the AVX-512 VL instruction set (C++ run)],
    [WITH_AVX_512_VL_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_AVX_512_VL_CPP_LINK])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [for the AVX-512 VL instruction set (all)],
    [WITH_AVX_512_VL],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_AVX_512_VL_C_RUN
        || (1
          && WYNA_AVX_512_VL_is_yes
          && WITH_AVX_512_VL_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_AVX_512_VL_CPP_RUN
        || (1
          && WYNA_AVX_512_VL_is_yes
          && WITH_AVX_512_VL_CPP_RUN_is_guess
        )
      )
    ])[

  case $][{WYNA_AVX_512_VL_is_yes?}$][{WITH_AVX_512_VL?} in 10)
    ]GATBPS_BARF([
      The AVX-512 VL instruction set is not available.
    ])[
  esac

}]])
