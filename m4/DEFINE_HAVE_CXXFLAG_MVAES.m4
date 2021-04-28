dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_MVAES], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_VAES]])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_F]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MAVX512F]])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -mvaes (compile 1)],
  [HAVE_CXXFLAG_MVAES_COMPILE_1],
  [-mvaes],
  [

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_VAES]])[

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
    __m512i x = _mm512_setzero_si512();
    x = _mm512_aesdec_epi128(x, x);
    x = _mm512_aesdeclast_epi128(x, x);
    x = _mm512_aesenc_epi128(x, x);
    x = _mm512_aesenclast_epi128(x, x);
    (void)x;
  ]]])[

old_CXXFLAGS=$CXXFLAGS

CXXFLAGS="$CXXFLAGS -mvaes"

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [CXXFLAGS += -mvaes (compile 2)],
  [HAVE_CXXFLAG_MVAES_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CXXFLAG_MVAES_COMPILE_1
  ])[
]AC_LANG_POP([C++])[

]GATBPS_ARG_WITH_BOOL(
  [CXXFLAGS += -mvaes (cross)],
  [WITH_CROSS_CXXFLAG_MVAES],
  [cross-cflag-mvaes],
  [no],
  [assume that CXXFLAGS += -mvaes is available on the host system when cross compiling],
  [assume that CXXFLAGS += -mvaes is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_RUN(
  [CXXFLAGS += -mvaes (run)],
  [HAVE_CXXFLAG_MVAES],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CXXFLAG_MVAES],
  [
    HAVE_CXXFLAG_MVAES_COMPILE_2
  ])[
]AC_LANG_POP([C++])[

case $HAVE_CXXFLAG_MVAES in
  0)
    CXXFLAGS=$old_CXXFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
