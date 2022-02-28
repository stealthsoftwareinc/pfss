dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_X86_32], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])[
  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]m4_pushdef(
    [prologue],
    [[
    ]])[

  ]m4_pushdef(
    [body],
    [[
      #if !(0 \
        || defined(_M_IX86) \
        || defined(__i386) \
        || defined(__i386__) \
        || defined(i386) \
      )
        #include <cause_a_compilation_error>
      #endif
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_LINK(
    [for the x86-32 instruction set (C link)],
    [HAVE_X86_32_C_LINK],
    prologue,
    body,
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
    ])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_LINK(
    [for the x86-32 instruction set (C++ link)],
    [HAVE_X86_32_CPP_LINK],
    prologue,
    body,
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [for the x86-32 instruction set (all)],
    [HAVE_X86_32],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || HAVE_X86_32_C_LINK
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || HAVE_X86_32_CPP_LINK
      )
    ])[

  ]GATBPS_FINISH_WYNA(
    [--with-x86-32],
    [WYNA_X86_32])[

  ]GATBPS_CHECK_EXPR(
    [whether to use the x86-32 instruction set (result)],
    [WITH_X86_32],
    [1
      && !WYNA_X86_32_is_no
      && HAVE_X86_32
    ])[

  case $][{WYNA_X86_32_is_yes?}$][{WITH_X86_32?} in 10)
    ]GATBPS_BARF([
      The x86-32 instruction set is not available.
    ])[
  esac

}]])
