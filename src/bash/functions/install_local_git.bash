#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

install_local_git() {
  orig_wd="${PWD}"
  make "$1"
  mkdir -p build-aux/local/bin
  tar -xzf "$1" -C build-aux/local/
  cd build-aux/local/git-*
    make configure
    ./configure --prefix="$orig_wd/build-aux/local/"
    make_j all
    make install
  cd "$orig_wd"

  PATH="${PWD}/build-aux/local/bin:${PATH}"
  export PATH
}; readonly -f install_local_git
