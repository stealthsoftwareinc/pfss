dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CFLAG_MAVX512VL], [[{

]AC_BEFORE([$0], [DEFINE_CFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_VL]])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_F]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CFLAG_MAVX512F]])[

]GATBPS_CHECK_CFLAG(
  [CFLAGS += -mavx512vl (compile 1)],
  [HAVE_CFLAG_MAVX512VL_COMPILE_1],
  [-mavx512vl],
  [

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_VL]])[

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_F]])[
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CFLAG_MAVX512F]])[

  ])[

]m4_pushdef(
  [prologue],
  [[[
    #include <immintrin.h>
  ]]])[

]m4_pushdef(
  [body],
  [[[
    __m256i x = _mm256_abs_epi64(_mm256_setzero_si256());
    (void)x;
  ]]])[

old_CFLAGS=$CFLAGS

CFLAGS="$CFLAGS -mavx512vl"

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_COMPILE(
  [CFLAGS += -mavx512vl (compile 2)],
  [HAVE_CFLAG_MAVX512VL_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CFLAG_MAVX512VL_COMPILE_1
  ])[
]AC_LANG_POP([C])[

]GATBPS_ARG_WITH_BOOL(
  [CFLAGS += -mavx512vl (cross)],
  [WITH_CROSS_CFLAG_MAVX512VL],
  [cross-cflag-mavx512vl],
  [no],
  [assume that CFLAGS += -mavx512vl is available on the host system when cross compiling],
  [assume that CFLAGS += -mavx512vl is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_RUN(
  [CFLAGS += -mavx512vl (run)],
  [HAVE_CFLAG_MAVX512VL],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CFLAG_MAVX512VL],
  [
    HAVE_CFLAG_MAVX512VL_COMPILE_2
  ])[
]AC_LANG_POP([C])[

case $HAVE_CFLAG_MAVX512VL in
  0)
    CFLAGS=$old_CFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
