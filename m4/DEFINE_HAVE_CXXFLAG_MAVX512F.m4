dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_MAVX512F], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_F]])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -mavx512f (compile 1)],
  [HAVE_CXXFLAG_MAVX512F_COMPILE_1],
  [-mavx512f],
  [
    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_F]])[
  ])[

]m4_pushdef(
  [prologue],
  [[[
    #include <immintrin.h>
  ]]])[

]m4_pushdef(
  [body],
  [[[
    __m512i x = _mm512_abs_epi32(_mm512_setzero_si512());
    (void)x;
  ]]])[

old_CXXFLAGS=$CXXFLAGS

CXXFLAGS="$CXXFLAGS -mavx512f"

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [CXXFLAGS += -mavx512f (compile 2)],
  [HAVE_CXXFLAG_MAVX512F_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CXXFLAG_MAVX512F_COMPILE_1
  ])[
]AC_LANG_POP([C++])[

]GATBPS_ARG_WITH_BOOL(
  [CXXFLAGS += -mavx512f (cross)],
  [WITH_CROSS_CXXFLAG_MAVX512F],
  [cross-cxxflag-mavx512f],
  [no],
  [assume that CXXFLAGS += -mavx512f is available on the host system when cross compiling],
  [assume that CXXFLAGS += -mavx512f is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_RUN(
  [CXXFLAGS += -mavx512f (run)],
  [HAVE_CXXFLAG_MAVX512F],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CXXFLAG_MAVX512F],
  [
    HAVE_CXXFLAG_MAVX512F_COMPILE_2
  ])[
]AC_LANG_POP([C++])[

case $HAVE_CXXFLAG_MAVX512F in
  0)
    CXXFLAGS=$old_CXXFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
