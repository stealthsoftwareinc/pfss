#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_pushd() {

  pushd "$@" >/dev/null

}; readonly -f sst_pushd
