#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_not_exist() {

  local path

  for path; do
    if [[ -e "$path" ]]; then
      path=$(sst_quote "$path")
      sst_barf "Path was expected to not exist: $path."
    fi
  done

}; readonly -f sst_expect_not_exist
