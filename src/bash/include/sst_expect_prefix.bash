#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_prefix() {

  declare path
  declare prefix
  declare x

  if (($# == 0)); then
    path=$(cat | sst_csf)
    sst_csf path
  elif (($# == 1)); then
    path=$1
  else
    sst_expect_argument_count $# 0-1
  fi
  readonly path

  prefix=$(sst_get_prefix "$path" | sst_csf)
  sst_csf prefix
  readonly prefix

  if [[ "$prefix" == "" ]]; then
    x=$(sst_smart_quote "$path")
    sst_barf "Path must have a nonempty prefix: $x"
  fi

  printf '%s\n' "$prefix"

}; readonly -f sst_expect_prefix
