#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_file() {

  local path

  for path; do
    if [[ -f "$path" ]]; then
      continue
    fi
    path=$(sst_smart_quote "$path")
    sst_barf "expected path to exist as a file: $path"
  done

}; readonly -f sst_expect_file
