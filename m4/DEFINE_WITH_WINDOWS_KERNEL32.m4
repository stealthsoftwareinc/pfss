dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_WINDOWS_KERNEL32], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_LIBS])

  GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])
  GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])
  GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])
  GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])
  GATBPS_SOFT_REQUIRE([DEFINE_WITH_WINDOWS])
  GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])

  GATBPS_FINISH_WYNA(
    [--with-windows-kernel32],
    [WYNA_WINDOWS_KERNEL32])

  GATBPS_PUSH_VAR([LIBS], ["-lkernel32 $][{LIBS-}"])

  m4_pushdef(
    [prologue],
    [[
      #include <windows.h>
    ]])

  m4_pushdef(
    [body],
    [[
      (void)GetLastError();
    ]])

  AC_LANG_PUSH([C])

  GATBPS_CHECK_LINK(
    [for the Windows kernel32 API (C link)],
    [WITH_WINDOWS_KERNEL32_C_LINK],
    prologue,
    body,
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
      && ]GATBPS_SOFT_VAR([WITH_WINDOWS])[
      && !WYNA_WINDOWS_KERNEL32_is_no
    ])

  GATBPS_CHECK_RUN(
    [for the Windows kernel32 API (C run)],
    [WITH_WINDOWS_KERNEL32_C_RUN],
    prologue,
    body,
    [yes],
    [WITH_WINDOWS_KERNEL32_C_LINK])

  AC_LANG_POP([C])

  AC_LANG_PUSH([C++])

  GATBPS_CHECK_LINK(
    [for the Windows kernel32 API (C++ link)],
    [WITH_WINDOWS_KERNEL32_CPP_LINK],
    prologue,
    body,
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      && ]GATBPS_SOFT_VAR([WITH_WINDOWS])[
      && !WYNA_WINDOWS_KERNEL32_is_no
    ])

  GATBPS_CHECK_RUN(
    [for the Windows kernel32 API (C++ run)],
    [WITH_WINDOWS_KERNEL32_CPP_RUN],
    prologue,
    body,
    [yes],
    [WITH_WINDOWS_KERNEL32_CPP_LINK])

  AC_LANG_POP([C++])

  m4_popdef([body])
  m4_popdef([prologue])

  GATBPS_CHECK_EXPR(
    [for the Windows kernel32 API (all)],
    [WITH_WINDOWS_KERNEL32],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_WINDOWS_KERNEL32_C_RUN
        || (1
          && WYNA_WINDOWS_KERNEL32_is_yes
          && WITH_WINDOWS_KERNEL32_C_RUN_is_guess
        )
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_WINDOWS_KERNEL32_CPP_RUN
        || (1
          && WYNA_WINDOWS_KERNEL32_is_yes
          && WITH_WINDOWS_KERNEL32_CPP_RUN_is_guess
        )
      )
    ])

  [

    case $][{WYNA_WINDOWS_KERNEL32_is_yes?}$][{WITH_WINDOWS_KERNEL32?} in 10)
      ]GATBPS_BARF([
        The Windows kernel32 API is not available.
      ])[
    esac

    if $][{WITH_WINDOWS_KERNEL32_sh?}; then
      ]GATBPS_KEEP_VAR([LIBS])[
    else
      ]GATBPS_POP_VAR([LIBS])[
    fi

  ]

}])
