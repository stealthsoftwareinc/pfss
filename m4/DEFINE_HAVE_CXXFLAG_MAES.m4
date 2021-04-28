dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_MAES], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MSSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MSSE2]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AES_NI]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE2]])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -maes (compile 1)],
  [HAVE_CXXFLAG_MAES_COMPILE_1],
  [-maes],
  [
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MSSE]])[
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MSSE2]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_AES_NI]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_SSE]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_SSE2]])[
  ])[

]m4_pushdef(
  [prologue],
  [[[
    #include <emmintrin.h> // SSE2
    #include <wmmintrin.h> // AES and PCLMULQDQ
  ]]])[

]m4_pushdef(
  [body],
  [[[
    __m128i x = _mm_setzero_si128();
    x = _mm_aesdec_si128(x, x);
    x = _mm_aesdeclast_si128(x, x);
    x = _mm_aesenc_si128(x, x);
    x = _mm_aesenclast_si128(x, x);
    x = _mm_aesimc_si128(x);
    x = _mm_aeskeygenassist_si128(x, 0);
    (void)x;
  ]]])[

old_CXXFLAGS=$CXXFLAGS

CXXFLAGS="$CXXFLAGS -maes"

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [CXXFLAGS += -maes (compile 2)],
  [HAVE_CXXFLAG_MAES_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CXXFLAG_MAES_COMPILE_1
  ])[
]AC_LANG_POP([C++])[

]GATBPS_ARG_WITH_BOOL(
  [CXXFLAGS += -maes (cross)],
  [WITH_CROSS_CXXFLAG_MAES],
  [cross-cxxflag-maes],
  [no],
  [assume that CXXFLAGS += -maes is available on the host system when cross compiling],
  [assume that CXXFLAGS += -maes is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_RUN(
  [CXXFLAGS += -maes (run)],
  [HAVE_CXXFLAG_MAES],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CXXFLAG_MAES],
  [
    HAVE_CXXFLAG_MAES_COMPILE_2
  ])[
]AC_LANG_POP([C++])[

case $HAVE_CXXFLAG_MAES in
  0)
    CXXFLAGS=$old_CXXFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
