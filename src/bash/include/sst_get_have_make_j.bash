#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_get_have_make_j() {

  # Bash >=4.2: declare -g sst_have_make_j

  if (($# > 0)); then
    sst_expect_argument_count $# 0
  fi

  if [[ "${sst_have_make_j+x}" ]]; then
    return
  fi

  if make -f /dev/null -j 1 / >/dev/null 2>&1; then
    sst_have_make_j=1
  else
    sst_have_make_j=
  fi

  readonly sst_have_make_j

}; readonly -f sst_get_have_make_j
