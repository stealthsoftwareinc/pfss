#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_distribute_if_not_dir() {

  local path

  for path; do
    if [[ ! -d "$path" ]]; then
      sst_am_distribute "$path"
    fi
  done

}; readonly -f sst_am_distribute_if_not_dir
