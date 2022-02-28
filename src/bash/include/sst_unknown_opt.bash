#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_unknown_opt() {

  local arg

  sst_expect_argument_count $# 1

  arg=$1
  readonly arg

  if ((${#arg} > 1)) && [[ "${arg:0:1}" == - ]]; then
    if [[ "${arg:1:1}" == - ]]; then
      sst_barf "unknown option: ${arg%%=*}"
    fi
    sst_barf "unknown option: ${arg:0:2}"
  fi

}; readonly -f sst_unknown_opt
