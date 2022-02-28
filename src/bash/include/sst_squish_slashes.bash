#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_squish_slashes() {

  declare    path
  declare    x

  if (($# == 0)); then
    path=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
    sst_csf path || sst_err_trap "$sst_last_command"
  elif (($# == 1)); then
    path=$1
  else
    sst_expect_argument_count $# 0-1
  fi
  readonly path

  x=${path//+('/')/'/'}
  if [[ "$path" == // || "$path" == //[!/]* ]]; then
    x=/$x
  fi
  readonly x

  printf '%s\n' "$x" || sst_err_trap "$sst_last_command"

}; readonly -f sst_squish_slashes
