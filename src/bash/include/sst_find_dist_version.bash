#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_find_dist_version() {

  sst_expect_argument_count $# 0

  if [[ -f build-aux/gatbps-gen-version.sh ]]; then
    sh build-aux/gatbps-gen-version.sh
    return
  fi

  sst_barf 'unable to find dist version'

}; readonly -f sst_find_dist_version
