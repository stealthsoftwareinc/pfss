#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_mkdir_p_only() {

  local path

  for path; do
    if [[ "$path" && ! -e "$path" ]]; then
      path=$(sst_dot_slash "$path") || sst_barf
      mkdir -p "$path" || sst_barf
      rmdir "$path" || sst_barf
    fi
  done

}; readonly -f sst_mkdir_p_only
