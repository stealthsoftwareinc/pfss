#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ag_include() {

  local file
  # Bash >=4.2: declare -g sst_ag_include_seen

  for file; do
    sst_expect_source_path "$file"
    if [[ "${sst_ag_include_seen= }" != *" $file "* ]]; then
      sst_ag_include_seen+="$file "
      sst_am_distribute_file $file
      . $file
    fi
  done

}; readonly -f sst_ag_include
