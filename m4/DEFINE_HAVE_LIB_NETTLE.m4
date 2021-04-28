dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_LIB_NETTLE], [[{

]AC_BEFORE([$0], [DEFINE_LIBS])[

]dnl begin_prerequisites
[

]AC_REQUIRE([DEFINE_CFLAGS])[
]AC_REQUIRE([DEFINE_CPPFLAGS])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_LIB_GMP]])[
]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_NETTLE]])[

]dnl end_prerequisites
[

old_LIBS=$LIBS

LIBS="-lnettle $LIBS"

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_LINK(
  [-lnettle],
  [HAVE_LIB_NETTLE],
  [[
    #include <nettle/aes.h>
    #include <stdint.h>
  ]],
  [[
    struct aes128_ctx ctx;
    uint8_t key[16] = {0};
    aes128_set_encrypt_key(&ctx, key);
  ]],
  [
    ]GATBPS_SOFT_VAR_DEP([[WITH_NETTLE]])[
  ])[
]AC_LANG_POP([C])[

case $HAVE_LIB_NETTLE in
  0)
    LIBS=$old_LIBS
  ;;
esac

:;}]])
