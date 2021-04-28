#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_popd() {

  popd "$@" >/dev/null

}; readonly -f sst_popd
