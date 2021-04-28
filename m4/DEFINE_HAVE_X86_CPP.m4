dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_X86_CPP], [[{ :

]AC_REQUIRE([DEFINE_HAVE_X86_C])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_X86]])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [x86 (C++)],
  [HAVE_X86_CPP],
  HAVE_X86_C_PROLOGUE,
  HAVE_X86_C_BODY,
  [
    ]GATBPS_SOFT_VAR_DEP([[WITH_X86]])[
  ])[
]AC_LANG_POP([C++])[

}]])
