dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_LIBS], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :
  GATBPS_REQUIRE([DEFINE_WITH_NETTLE])
  GATBPS_REQUIRE([DEFINE_WITH_WINDOWS_BCRYPT])
  GATBPS_REQUIRE([DEFINE_WITH_WINDOWS_KERNEL32])
}])
