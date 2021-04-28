#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_unknown_get_distro_version() {

  sst_expect_argument_count $# 0

  echo unknown

}; readonly -f sst_unknown_get_distro_version
