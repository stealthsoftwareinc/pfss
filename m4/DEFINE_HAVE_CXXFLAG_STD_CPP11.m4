dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_CXXFLAG_STD_CPP11], [[{

]AC_BEFORE([$0], [DEFINE_CXXFLAGS])[

]GATBPS_CHECK_CXXFLAG(
  [CXXFLAGS += -std=c++11],
  [HAVE_CXXFLAG_STD_CPP11],
  [-std=c++11])[

case $HAVE_CXXFLAG_STD_CPP11 in
  1)
    CXXFLAGS="$CXXFLAGS -std=c++11"
  ;;
esac

:;}]])
