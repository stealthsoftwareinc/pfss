dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_AVX_512_F], [[{

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_F]])[
]AC_REQUIRE([DEFINE_HAVE_CFLAG_MAVX512F])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MAVX512F]])[

]GATBPS_CHECK_VARS(
  [AVX-512 F],
  [HAVE_AVX_512_F],
  [

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_F]])[
    HAVE_CFLAG_MAVX512F
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MAVX512F]])[

  ])[

]GATBPS_ARG_WITH_BOOL(
  [insistence to use AVX-512 F],
  [WITH_AVX_512_F_OR_DIE],
  [avx-512-f-or-die],
  [no],
  [demand the use of AVX-512 F],
  [defer to --with-avx-512-f])[

case $WITH_AVX_512_F_OR_DIE,$HAVE_AVX_512_F in
  1,0)
    ]GATBPS_MSG_ERROR([
      You specified --with-avx-512-f-or-die, but AVX-512 F is not available.
    ])[
  ;;
esac

:;}]])
