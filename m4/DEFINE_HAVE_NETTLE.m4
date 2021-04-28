dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_NETTLE], [[{

]dnl begin_prerequisites
[

]AC_REQUIRE([DEFINE_HAVE_LIB_NETTLE])[
]AC_REQUIRE([DEFINE_WITH_NETTLE])[

]dnl end_prerequisites
[

]GATBPS_CHECK_VARS(
  [Nettle],
  [HAVE_NETTLE],
  [
    HAVE_LIB_NETTLE
    WITH_NETTLE
  ])[

]GATBPS_ARG_WITH_BOOL(
  [insistence to use Nettle],
  [WITH_NETTLE_OR_DIE],
  [nettle-or-die],
  [no],
  [demand the use of Nettle],
  [defer to --with-nettle])[

case $WITH_NETTLE_OR_DIE,$HAVE_NETTLE in
  1,0)
    ]GATBPS_MSG_ERROR([
      You specified --with-nettle-or-die, but Nettle is not available.
    ])[
  ;;
esac

:;}]])
