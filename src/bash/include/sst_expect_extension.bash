#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_extension() {

  local extension
  local path

  sst_expect_argument_count $# 2

  path=$1
  readonly path

  extension=$2
  readonly extension

  if [[ "$path" != *[!/]"$extension" ]]; then
    sst_barf 'path must end in %s: %s' "$extension" "$path"
  fi

}; readonly -f sst_expect_extension
