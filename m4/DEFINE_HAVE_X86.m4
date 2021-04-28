dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_X86], [[{ :

]AC_REQUIRE([DEFINE_HAVE_X86_C])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_X86_CPP]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_X86]])[

]GATBPS_CHECK_VARS(
  [x86 CPU features],
  [HAVE_X86],
  [
    HAVE_X86_C
    ]GATBPS_SOFT_VAR_DEP([[HAVE_X86_CPP]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_X86]])[
  ])[

]GATBPS_ARG_WITH_BOOL(
  [insistence to use x86 CPU features],
  [WITH_X86_OR_DIE],
  [x86-or-die],
  [no],
  [demand the use of x86 CPU features],
  [defer to --with-x86])[

case $WITH_X86_OR_DIE,$HAVE_X86 in
  1,0)
    ]GATBPS_MSG_ERROR([
      You specified --with-x86-or-die, but x86 CPU features are not
      available.
    ])[
  ;;
esac

}]])
