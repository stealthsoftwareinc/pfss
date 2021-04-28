dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CFLAG_MVAES], [[{

]AC_BEFORE([$0], [DEFINE_CFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_VAES]])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_AVX_512_F]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CFLAG_MAVX512F]])[

]GATBPS_CHECK_CFLAG(
  [CFLAGS += -mvaes (compile 1)],
  [HAVE_CFLAG_MVAES_COMPILE_1],
  [-mvaes],
  [

    ]GATBPS_SOFT_VAR_DEP([[WITH_AVX_512_VAES]])[

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
    __m512i x = _mm512_setzero_si512();
    x = _mm512_aesdec_epi128(x, x);
    x = _mm512_aesdeclast_epi128(x, x);
    x = _mm512_aesenc_epi128(x, x);
    x = _mm512_aesenclast_epi128(x, x);
    (void)x;
  ]]])[

old_CFLAGS=$CFLAGS

CFLAGS="$CFLAGS -mvaes"

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_COMPILE(
  [CFLAGS += -mvaes (compile 2)],
  [HAVE_CFLAG_MVAES_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CFLAG_MVAES_COMPILE_1
  ])[
]AC_LANG_POP([C])[

]GATBPS_ARG_WITH_BOOL(
  [CFLAGS += -mvaes (cross)],
  [WITH_CROSS_CFLAG_MVAES],
  [cross-cflag-mvaes],
  [no],
  [assume that CFLAGS += -mvaes is available on the host system when cross compiling],
  [assume that CFLAGS += -mvaes is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_RUN(
  [CFLAGS += -mvaes (run)],
  [HAVE_CFLAG_MVAES],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CFLAG_MVAES],
  [
    HAVE_CFLAG_MVAES_COMPILE_2
  ])[
]AC_LANG_POP([C])[

case $HAVE_CFLAG_MVAES in
  0)
    CFLAGS=$old_CFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
