dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_SSE], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])[
  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]GATBPS_FINISH_WYNA(
    [--with-sse],
    [WYNA_SSE])[

  ]m4_pushdef(
    [prologue],
    [[
      #include <xmmintrin.h>
    ]])[

  ]m4_pushdef(
    [body],
    [[
      _mm_sfence();
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_LINK(
    [for the SSE instruction set (C link)],
    [WITH_SSE_C_LINK],
    prologue,
    body,
    [1
      && !WYNA_SSE_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for the SSE instruction set (C run)],
    [WITH_SSE_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_SSE_C_LINK])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_LINK(
    [for the SSE instruction set (C++ link)],
    [WITH_SSE_CPP_LINK],
    prologue,
    body,
    [1
      && !WYNA_SSE_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])[

  ]GATBPS_CHECK_RUN(
    [for the SSE instruction set (C++ run)],
    [WITH_SSE_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_SSE_CPP_LINK])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [for the SSE instruction set (all)],
    [WITH_SSE],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_SSE_C_RUN
        || (1
          && WYNA_SSE_is_yes
          && WITH_SSE_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_SSE_CPP_RUN
        || (1
          && WYNA_SSE_is_yes
          && WITH_SSE_CPP_RUN_is_guess
        )
      )
    ])[

  case $][{WYNA_SSE_is_yes?}$][{WITH_SSE?} in 10)
    ]GATBPS_BARF([
      The SSE instruction set is not available.
    ])[
  esac

}]])
