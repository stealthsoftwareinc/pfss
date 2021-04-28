#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ac_include() {

  local file
  # Bash >=4.2: declare -g sst_ac_include_seen

  for file; do
    sst_expect_source_path "$file"
    if [[ "${sst_ac_include_seen= }" != *" $file "* ]]; then
      sst_ac_include_seen+="$file "
      sst_ac_append <<<"m4_include([$file])"
    fi
  done

}; readonly -f sst_ac_include
