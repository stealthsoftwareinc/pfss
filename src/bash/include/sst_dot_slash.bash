#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_dot_slash() {

  declare    path

  if (($# == 0)); then
    path=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
    sst_csf path || sst_err_trap "$sst_last_command"
  elif (($# == 1)); then
    path=$1
  else
    sst_expect_argument_count $# 0-1
  fi

  if [[ ! "$path" ]]; then
    sst_barf "A path was empty."
  fi

  if [[ "$path" == [!/]* ]]; then
    path=./$path
  fi

  printf '%s\n' "$path" || sst_err_trap "$sst_last_command"

}; readonly -f sst_dot_slash
