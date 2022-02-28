#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_maybe_file() {

  local path

  for path; do
    if [[ -f "$path" || ! -e "$path" ]]; then
      continue
    fi
    path=$(sst_smart_quote "$path")
    sst_barf \
      "expected path to either exist as a file or not exist: $path" \
    ;
  done

}; readonly -f sst_expect_maybe_file
