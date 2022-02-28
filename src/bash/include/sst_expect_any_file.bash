#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_any_file() {

  local path
  local paths

  for path; do
    if [[ -f "$path" ]]; then
      return
    fi
  done

  paths=
  for path; do
    paths+=${paths:+ }$(sst_smart_quote "$path")
  done
  sst_barf "expected at least one path to exist as a file: $paths"

}; readonly -f sst_expect_any_file
