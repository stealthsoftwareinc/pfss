dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_WEXTRA], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -Wextra],
  [HAVE_CXXFLAG_WEXTRA],
  [-Wextra])[

case $HAVE_CXXFLAG_WEXTRA in
  1)
    CXXFLAGS="$CXXFLAGS -Wextra"
  ;;
esac

:;}]])
