#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_am_distribute_file() {

  local file
  local i
  local n
  # Bash >=4.2: declare -g sst_am_distribute_i

  sst_expect_no_subshell

  # n should be hardcoded to an integer value between 1 and k+1
  # inclusive, where k is the number from the highest numbered
  # GATBPS_DISTFILES_k target in build-aux/gatbps.am.
  n=100
  readonly n

  for file; do
    i=${sst_am_distribute_i-0}
    sst_am_var_add_unique_file GATBPS_DISTFILES_$i "$file"
    i=$(((i + 1) % n))
    sst_am_distribute_i=$i
  done

}; readonly -f sst_am_distribute_file
