dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_MSSE2], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MSSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_SSE2]])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -msse2 (compile 1)],
  [HAVE_CXXFLAG_MSSE2_COMPILE_1],
  [-msse2],
  [
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MSSE]])[
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

old_CXXFLAGS=$CXXFLAGS

CXXFLAGS="$CXXFLAGS -msse2"

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [CXXFLAGS += -msse2 (compile 2)],
  [HAVE_CXXFLAG_MSSE2_COMPILE_2],
  prologue,
  body,
  [
    HAVE_CXXFLAG_MSSE2_COMPILE_1
  ])[
]AC_LANG_POP([C++])[

]GATBPS_ARG_WITH_BOOL(
  [CXXFLAGS += -msse2 (cross)],
  [WITH_CROSS_CXXFLAG_MSSE2],
  [cross-cxxflag-msse2],
  [no],
  [assume that CXXFLAGS += -msse2 is available on the host system when cross compiling],
  [assume that CXXFLAGS += -msse2 is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_RUN(
  [CXXFLAGS += -msse2 (run)],
  [HAVE_CXXFLAG_MSSE2],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CXXFLAG_MSSE2],
  [
    HAVE_CXXFLAG_MSSE2_COMPILE_2
  ])[
]AC_LANG_POP([C++])[

case $HAVE_CXXFLAG_MSSE2 in
  0)
    CXXFLAGS=$old_CXXFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
