dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_JNI], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_SOFT_REQUIRE([DEFINE_CFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CXXFLAGS])[
  ]GATBPS_REQUIRE([DEFINE_DEDUCE_WYNAS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]GATBPS_FINISH_WYNA(
    [--with-jni],
    [WYNA_JNI])[

  ]m4_pushdef(
    [prologue],
    [[
      #include <jni.h>
    ]])[

  ]m4_pushdef(
    [body],
    [[
      JNIEnv const * const env = 0;
      (void)env;
    ]])[

  ]AC_LANG_PUSH([C])[

  ]GATBPS_CHECK_COMPILE(
    [for the Java Native Interface (C compile)],
    [WITH_JNI_C_COMPILE],
    prologue,
    body,
    [1
      && !WYNA_JNI_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
    ])[

  ]AC_LANG_POP([C])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_COMPILE(
    [for the Java Native Interface (C++ compile)],
    [WITH_JNI_CPP_COMPILE],
    prologue,
    body,
    [1
      && !WYNA_JNI_is_no
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])[

  ]AC_LANG_POP([C++])[

  ]m4_popdef([body])[
  ]m4_popdef([prologue])[

  ]GATBPS_CHECK_EXPR(
    [for the Java Native Interface (all)],
    [WITH_JNI],
    [(1
      ) && (0
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_C_AUTOTOOLS])[
        || WITH_JNI_C_COMPILE
      ) && (0
        || !]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
        || WITH_JNI_CPP_COMPILE
      )
    ])[

  case $][{WYNA_JNI_is_yes?}$][{WITH_JNI?} in 10)
    ]GATBPS_BARF([
      The Java Native Interface is not available.
    ])[
  esac

}]])
