dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CFLAG_MARCH_ARM_CRYPTO], [[{

]AC_BEFORE([$0], [DEFINE_CFLAGS])[

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

old_CFLAGS=$CFLAGS

set x $CFLAGS
shift
CFLAGS=
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
      CFLAGS="$CFLAGS "`
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
      CFLAGS="$CFLAGS $x"
    ;;
  esac
done

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_COMPILE(
  [CFLAGS[-march=arm*] += +crypto (compile)],
  [HAVE_CFLAG_MARCH_ARM_CRYPTO_COMPILE],
  prologue,
  body,
  [
    ]GATBPS_SOFT_VAR_DEP([[WITH_ARM_CRYPTO]])[
  ])[
]AC_LANG_POP([C])[

]GATBPS_ARG_WITH_BOOL(
  [CFLAGS[-march=arm*] += +crypto (cross)],
  [WITH_CROSS_CFLAG_MARCH_ARM_CRYPTO],
  [cross-cflag-march-arm-crypto],
  [no],
  [assume that CFLAGS[-march=arm*] += +crypto is available on the host system when cross compiling],
  [assume that CFLAGS[-march=arm*] += +crypto is unavailable on the host system when cross compiling])[

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_RUN(
  [CFLAGS[-march=arm*] += +crypto (run)],
  [HAVE_CFLAG_MARCH_ARM_CRYPTO],
  prologue,
  body,
  [gatbps_cv_WITH_CROSS_CFLAG_MARCH_ARM_CRYPTO],
  [
    HAVE_CFLAG_MARCH_ARM_CRYPTO_COMPILE
  ])[
]AC_LANG_POP([C])[

case $HAVE_CFLAG_MARCH_ARM_CRYPTO in
  0)
    CFLAGS=$old_CFLAGS
  ;;
esac

]m4_popdef([body])[
]m4_popdef([prologue])[

:;}]])
