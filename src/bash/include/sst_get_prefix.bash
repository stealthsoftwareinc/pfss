#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_get_prefix() {

  sst_expect_argument_count $# 0-1
  local x
  if (($# == 0)); then
    x=$(cat | sst_csf)
    sst_csf x
  else
    x=$1
  fi
  if [[ "$x" == */* ]]; then
    x=${x%/*}/
  else
    x=
  fi
  printf '%s\n' "$x"

}; readonly -f sst_get_prefix
