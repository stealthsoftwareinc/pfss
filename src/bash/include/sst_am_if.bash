#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_if() {

  # Bash >=4.2: declare -g -a sst_am_if_vars
  declare n

  sst_expect_argument_count $# 1

  sst_expect_basic_identifier "${1#!}"

  if [[ "${sst_am_if_vars+x}" ]]; then
    n=${#sst_am_if_vars[@]}
  else
    n=0
  fi
  readonly n

  sst_am_if_vars[n]=$1

  sst_am_append <<<"if $1"

}; readonly -f sst_am_if
