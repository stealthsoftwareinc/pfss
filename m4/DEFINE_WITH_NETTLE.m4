dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_NETTLE], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_LIBS])

  GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])
  GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])
  GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])
  GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])
  GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])

  GATBPS_FINISH_WYNA(
    [--with-nettle],
    [WYNA_NETTLE])

  GATBPS_PUSH_VAR([LIBS], ["-lnettle $][{LIBS-}"])

  m4_pushdef(
    [prologue],
    [[
      #include <nettle/sha1.h>
    ]])

  m4_pushdef(
    [body],
    [[
      struct sha1_ctx ctx;
      sha1_init(&ctx);
    ]])

  AC_LANG_PUSH([C])

  GATBPS_CHECK_LINK(
    [for the Nettle nettle library (C link)],
    [WITH_NETTLE_C_LINK],
    prologue,
    body,
    [1
      && !WYNA_NETTLE_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
    ])

  GATBPS_CHECK_RUN(
    [for the Nettle nettle library (C run)],
    [WITH_NETTLE_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_NETTLE_C_LINK])

  AC_LANG_POP([C])

  AC_LANG_PUSH([C++])

  GATBPS_CHECK_LINK(
    [for the Nettle nettle library (C++ link)],
    [WITH_NETTLE_CPP_LINK],
    prologue,
    body,
    [1
      && !WYNA_NETTLE_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])

  GATBPS_CHECK_RUN(
    [for the Nettle nettle library (C++ run)],
    [WITH_NETTLE_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_NETTLE_CPP_LINK])

  AC_LANG_POP([C++])

  m4_popdef([body])
  m4_popdef([prologue])

  GATBPS_CHECK_EXPR(
    [for the Nettle nettle library (all)],
    [WITH_NETTLE],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_NETTLE_C_RUN
        || (1
          && WYNA_NETTLE_is_yes
          && WITH_NETTLE_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_NETTLE_CPP_RUN
        || (1
          && WYNA_NETTLE_is_yes
          && WITH_NETTLE_CPP_RUN_is_guess
        )
      )
    ])

  [

    case $][{WYNA_NETTLE_is_yes?}$][{WITH_NETTLE?} in 10)
      ]GATBPS_BARF([
        The Nettle nettle library is not available.
      ])[
    esac

    if $][{WITH_NETTLE_sh?}; then
      ]GATBPS_KEEP_VAR([LIBS])[
    else
      ]GATBPS_POP_VAR([LIBS])[
    fi

  ]

}])
