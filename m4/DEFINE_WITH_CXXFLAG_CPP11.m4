dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_CXXFLAG_CPP11], [[
]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
{ :

  ]GATBPS_BEFORE([$0], [DEFINE_CXXFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_CPPFLAGS])[
  ]GATBPS_SOFT_REQUIRE([DEFINE_WITH_BUILD_GROUPS])[

  ]AC_LANG_PUSH([C++])[

  ]GATBPS_CHECK_COMPILE(
    [whether C++11 is enabled (1)],
    [WITH_CXXFLAG_CPP11_1],
    [],
    [
      #if __cplusplus < 201103L
        #include <cause_a_compilation_error>
      #endif
    ],
    [1
      && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
    ])[

  ]AC_LANG_POP([C++])[

  if $][{WITH_CXXFLAG_CPP11_1_sh?}; then

    ]GATBPS_CHECK_EXPR(
      [whether C++11 is enabled (2)],
      [WITH_CXXFLAG_CPP11_2],
      [1])[

  else

    ]GATBPS_PUSH_VAR([CXXFLAGS], ["-std=c++11 $][{CXXFLAGS-}"])[

    ]AC_LANG_PUSH([C++])[

    ]GATBPS_CHECK_COMPILE(
      [whether C++11 is enabled (2)],
      [WITH_CXXFLAG_CPP11_2],
      [],
      [
        #if __cplusplus < 201103L
          #include <cause_a_compilation_error>
        #endif
      ],
      [1
        && ]GATBPS_SOFT_VAR([WITH_BUILD_GROUP_CPP_AUTOTOOLS])[
      ])[

    ]AC_LANG_POP([C++])[

    if $][{WITH_CXXFLAG_CPP11_2_sh?}; then
      :
    else
      ]GATBPS_BARF([
        C++11 is not available.
      ])[
    fi

    ]GATBPS_KEEP_VAR([CXXFLAGS])[

  fi

  ]GATBPS_CHECK_EXPR(
    [whether C++11 is enabled (3)],
    [WITH_CXXFLAG_CPP11],
    [WITH_CXXFLAG_CPP11_2])[

}]])
