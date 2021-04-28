#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_find_dist_version() {

  sst_expect_argument_count $# 0

  if [[ -f build-aux/VERSION.sh ]]; then
    sh build-aux/VERSION.sh
    return
  fi

  sst_barf 'unable to find dist version'

}; readonly -f sst_find_dist_version
