#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_find_dist_date() {

  sst_expect_argument_count $# 0

  if [[ -f build-aux/gatbps-gen-date.sh ]]; then
    sh build-aux/gatbps-gen-date.sh
    return
  fi

  date -u '+%Y-%m-%d'

}; readonly -f sst_find_dist_date
