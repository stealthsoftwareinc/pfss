#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_distribute() {

  local i
  local n
  local path
  # Bash >=4.2: declare -g sst_am_distribute_i
  # Bash >=4.2: declare -g sst_am_distribute_seen

  sst_expect_not_subshell

  # n should be hardcoded to an integer value between 1 and k+1
  # inclusive, where k is the number of the highest numbered
  # GATBPS_DISTFILES_k target in build-aux/gatbps.am.
  n=100
  readonly n

  for path; do
    sst_expect_source_path "$path"
    if [[ ! -f $path && ! -d $path && -e $path ]]; then
      path=$(sst_smart_quote $path)
      sst_barf \
        "path must either exist as a file," \
        "exist as a directory, or not exist: $path" \
      ;
    fi
    if [[ "${sst_am_distribute_seen= }" == *" $path "* ]]; then
      continue
    fi
    sst_am_distribute_seen+="$path "
    i=${sst_am_distribute_i-0}
    sst_am_var_add GATBPS_DISTFILES_$i $path
    sst_am_distribute_i=$(((i + 1) % n))
  done

}; readonly -f sst_am_distribute
