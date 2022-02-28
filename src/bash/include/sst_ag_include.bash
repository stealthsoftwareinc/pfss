#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_ag_include() {

  local file
  # Bash >=4.2: declare -g sst_ag_include_seen

  for file; do
    sst_expect_source_path "$file"
    if [[ "${sst_ag_include_seen= }" != *" $file "* ]]; then
      sst_info "processing $file"
      sst_ag_include_seen+="$file "
      sst_am_distribute $file
      . $file
    fi
  done

}; readonly -f sst_ag_include
