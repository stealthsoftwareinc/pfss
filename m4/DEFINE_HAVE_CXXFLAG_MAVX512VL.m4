dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_MAVX512VL], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_VL]])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_F]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MAVX512F]])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -mavx512vl (compile 1)],
  [HAVE_CXXFLAG_MAVX512VL_COMPILE_1],
  [-mavx512vl],
  [

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_VL]])[

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_F]])[
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MAVX512F]])[

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

old_CXXFLAGS=$CXXFLAGS

CXXFLAGS="$CXXFLAGS -mavx512vl"

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [CXXFLAGS += -mavx512vl (compile 2)],
  [HAVE_CXXFLAG_MAVX512VL_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CXXFLAG_MAVX512VL_COMPILE_1
  ])[
]AC_LANG_POP([C++])[

]GATBPS_ARG_WITH_BOOL(
  [CXXFLAGS += -mavx512vl (cross)],
  [WITH_CROSS_CXXFLAG_MAVX512VL],
  [cross-cxxflag-mavx512vl],
  [no],
  [assume that CXXFLAGS += -mavx512vl is available on the host system when cross compiling],
  [assume that CXXFLAGS += -mavx512vl is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_RUN(
  [CXXFLAGS += -mavx512vl (run)],
  [HAVE_CXXFLAG_MAVX512VL],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CXXFLAG_MAVX512VL],
  [
    HAVE_CXXFLAG_MAVX512VL_COMPILE_2
  ])[
]AC_LANG_POP([C++])[

case $HAVE_CXXFLAG_MAVX512VL in
  0)
    CXXFLAGS=$old_CXXFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
