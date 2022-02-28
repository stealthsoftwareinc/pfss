#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_make_j() {

  # Bash >=4.2: declare -g sst_have_make_j

  sst_get_have_make_j
  if ((sst_have_make_j)); then
    sst_get_max_procs
    make -j $sst_max_procs "$@"
  else
    make "$@"
  fi

}; readonly -f sst_make_j
