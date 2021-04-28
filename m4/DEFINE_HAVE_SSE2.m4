dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_SSE2], [[{

]AC_REQUIRE([DEFINE_HAVE_CFLAG_MSSE2])[
]AC_REQUIRE([DEFINE_WITH_SSE2])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CFLAG_MSSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MSSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MSSE2]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE2]])[

]GATBPS_CHECK_VARS(
  [SSE2],
  [HAVE_SSE2],
  [
    HAVE_CFLAG_MSSE2
    WITH_SSE2
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CFLAG_MSSE]])[
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MSSE]])[
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MSSE2]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_SSE]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_SSE2]])[
  ])[

]GATBPS_ARG_WITH_BOOL(
  [insistence to use SSE2],
  [WITH_SSE2_OR_DIE],
  [sse2-or-die],
  [no],
  [demand the use of SSE2],
  [defer to --with-sse2])[

case $WITH_SSE2_OR_DIE,$HAVE_SSE2 in
  1,0)
    ]GATBPS_MSG_ERROR([
      You specified --with-sse2-or-die, but SSE2 is not available.
    ])[
  ;;
esac

:;}]])
