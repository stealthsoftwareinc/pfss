dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_WALL], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -Wall],
  [HAVE_CXXFLAG_WALL],
  [-Wall])[

case $HAVE_CXXFLAG_WALL in
  1)
    CXXFLAGS="$CXXFLAGS -Wall"
  ;;
esac

:;}]])
