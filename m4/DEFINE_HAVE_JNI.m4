dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_JNI], [[{

]AC_REQUIRE([DEFINE_HAVE_C_INCLUDE_JNI_H])[
]AC_REQUIRE([DEFINE_WITH_JNI])[
]GATBPS_SOFT_REQUIRE([[DEFINE_HAVE_HAVE_CPP_INCLUDE_JNI_H]])[

]GATBPS_CHECK_VARS(
  [JNI],
  [HAVE_JNI],
  [
    HAVE_C_INCLUDE_JNI_H
    WITH_JNI
    ]GATBPS_SOFT_VAR_DEP([[HAVE_HAVE_CPP_INCLUDE_JNI_H]])[
  ])[

]GATBPS_ARG_WITH_BOOL(
  [insistence to use JNI],
  [WITH_JNI_OR_DIE],
  [jni-or-die],
  [no],
  [demand the use of JNI],
  [defer to --with-jni])[

case $WITH_JNI_OR_DIE,$HAVE_JNI in
  1,0)
    ]GATBPS_MSG_ERROR([
      You specified --with-jni-or-die, but JNI is not available.
    ])[
  ;;
esac

:;}]])
