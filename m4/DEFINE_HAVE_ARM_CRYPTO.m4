dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_ARM_CRYPTO], [[{

]AC_REQUIRE([DEFINE_HAVE_CFLAG_MARCH_ARM_CRYPTO])[
]AC_REQUIRE([DEFINE_WITH_ARM_CRYPTO])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_CXXFLAG_MARCH_ARM_CRYPTO]])[

]GATBPS_CHECK_VARS(
  [ARM Crypto],
  [HAVE_ARM_CRYPTO],
  [
    HAVE_CFLAG_MARCH_ARM_CRYPTO
    WITH_ARM_CRYPTO
    ]GATBPS_SOFT_VAR_DEP([[HAVE_CXXFLAG_MARCH_ARM_CRYPTO]])[
  ])[

]GATBPS_ARG_WITH_BOOL(
  [insistence to use ARM Crypto],
  [WITH_ARM_CRYPTO_OR_DIE],
  [arm-crypto-or-die],
  [no],
  [demand the use of ARM Crypto],
  [defer to --with-arm-crypto])[

case $WITH_ARM_CRYPTO_OR_DIE,$HAVE_ARM_CRYPTO in
  1,0)
    ]GATBPS_MSG_ERROR([
      You specified --with-arm-crypto-or-die, but ARM Crypto is not available.
    ])[
  ;;
esac

:;}]])
