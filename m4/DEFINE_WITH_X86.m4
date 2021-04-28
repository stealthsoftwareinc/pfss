dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_WITH_X86], [[{ :

]GATBPS_ARG_WITH_BOOL(
  [permission to use x86 CPU features],
  [WITH_X86],
  [x86],
  [yes],
  [permit the use of x86 CPU features],
  [forbid the use of x86 CPU features])[

}]])
