dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_JNI], [[{

]GATBPS_ARG_WITH_BOOL(
  [permission to use JNI],
  [WITH_JNI],
  [jni],
  [yes],
  [permit the use of JNI],
  [forbid the use of JNI])[

:;}]])
