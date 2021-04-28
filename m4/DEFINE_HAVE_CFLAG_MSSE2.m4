dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CFLAG_MSSE2], [[{

]AC_BEFORE([$0], [DEFINE_CFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CFLAG_MSSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE2]])[

]GATBPS_CHECK_CFLAG(
  [CFLAGS += -msse2 (compile 1)],
  [HAVE_CFLAG_MSSE2_COMPILE_1],
  [-msse2],
  [
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CFLAG_MSSE]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_SSE]])[
    ]GATBPS_SOFT_VAR_DEP([[WITH_SSE2]])[
  ])[

]m4_pushdef(
  [prologue],
  [[[
    #include <emmintrin.h> // SSE2
  ]]])[

]m4_pushdef(
  [body],
  [[[
    __m128i x = _mm_setzero_si128();
    (void)x;
  ]]])[

old_CFLAGS=$CFLAGS

CFLAGS="$CFLAGS -msse2"

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_COMPILE(
  [CFLAGS += -msse2 (compile 2)],
  [HAVE_CFLAG_MSSE2_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CFLAG_MSSE2_COMPILE_1
  ])[
]AC_LANG_POP([C])[

]GATBPS_ARG_WITH_BOOL(
  [CFLAGS += -msse2 (cross)],
  [WITH_CROSS_CFLAG_MSSE2],
  [cross-cflag-msse2],
  [no],
  [assume that CFLAGS += -msse2 is available on the host system when cross compiling],
  [assume that CFLAGS += -msse2 is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_RUN(
  [CFLAGS += -msse2 (run)],
  [HAVE_CFLAG_MSSE2],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CFLAG_MSSE2],
  [
    HAVE_CFLAG_MSSE2_COMPILE_2
  ])[
]AC_LANG_POP([C])[

case $HAVE_CFLAG_MSSE2 in
  0)
    CFLAGS=$old_CFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
