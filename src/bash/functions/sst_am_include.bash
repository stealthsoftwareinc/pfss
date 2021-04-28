#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_am_include() {

  local file
  # Bash >=4.2: declare -g sst_am_include_seen

  for file; do
    sst_expect_source_path "$file"
    if [[ "${sst_am_include_seen= }" != *" $file "* ]]; then
      sst_am_include_seen+="$file "
      sst_am_append <<<"include \$(srcdir)/$file"
    fi
  done

}; readonly -f sst_am_include
