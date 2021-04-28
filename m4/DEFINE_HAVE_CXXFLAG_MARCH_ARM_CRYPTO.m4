dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_MARCH_ARM_CRYPTO], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_ARM_CRYPTO]])[

]m4_pushdef(
  [prologue],
  [[[
    #include <arm_neon.h>
  ]]])[

]m4_pushdef(
  [body],
  [[[
    uint8x16_t x = {0};
    x = vaeseq_u8(x, x);
    (void)x;
  ]]])[

old_CXXFLAGS=$CXXFLAGS

set x $CXXFLAGS
shift
CXXFLAGS=
for x; do
  case $x in
    -march=native | -mcpu=native)
    ;;
    -march=* | -mcpu=*)
      x=$x+crypto
    ;;
  esac
  case $x in
    *[!%+,./0-9:=@A-Z^_a-z~-]*)
      CXXFLAGS="$CXXFLAGS "`
        sed "
          s/'/'\\\\''/g
          1s/^/'/
          \$s/\$/'/
        " <<EOF
$x
EOF
      ` || exit $?
    ;;
    *)
      CXXFLAGS="$CXXFLAGS $x"
    ;;
  esac
done

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_COMPILE(
  [CXXFLAGS[-march=arm*] += +crypto (compile)],
  [HAVE_CXXFLAG_MARCH_ARM_CRYPTO_COMPILE],
  prologue,
  body,
  [
    ]GATBPS_SOFT_VAR_DEP([[WITH_ARM_CRYPTO]])[
  ])[
]AC_LANG_POP([C++])[

]GATBPS_ARG_WITH_BOOL(
  [CXXFLAGS[-march=arm*] += +crypto (cross)],
  [WITH_CROSS_CXXFLAG_MARCH_ARM_CRYPTO],
  [cross-cxxflag-march-arm-crypto],
  [no],
  [assume that CXXFLAGS[-march=arm*] += +crypto is available on the host system when cross compiling],
  [assume that CXXFLAGS[-march=arm*] += +crypto is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C++])[
]GATBPS_CHECK_RUN(
  [CXXFLAGS[-march=arm*] += +crypto (run)],
  [HAVE_CXXFLAG_MARCH_ARM_CRYPTO],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CXXFLAG_MARCH_ARM_CRYPTO],
  [
    HAVE_CXXFLAG_MARCH_ARM_CRYPTO_COMPILE
  ])[
]AC_LANG_POP([C++])[

case $HAVE_CXXFLAG_MARCH_ARM_CRYPTO in
  0)
    CXXFLAGS=$old_CXXFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
