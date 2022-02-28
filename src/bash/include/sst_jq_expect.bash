#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_jq_expect() {

  local condition
  local result

  sst_expect_argument_count $# 1-

  condition=$1
  readonly condition

  result=$(jq " $condition")
  readonly result

  if [[ "$result" == true ]]; then
    :
  elif [[ "$result" == false ]]; then
    shift
    sst_barf "$@"
  else
    condition=$(sst_quote "$condition")
    result=$(sst_quote "$result")
    sst_barf '<condition> = %s output %s instead of true or false' "$condition" "$result"
  fi

}; readonly -f sst_jq_expect
