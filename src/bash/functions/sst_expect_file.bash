#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_expect_file() {

  local file

  for file; do
    sst_expect_any_file "$file"
  done

}; readonly -f sst_expect_file
