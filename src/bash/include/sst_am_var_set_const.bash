#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_var_set_const() {

  # Bash >=4.2: declare -g -A sst_am_var_const

  sst_am_var_set "$@"
  sst_am_var_const[$1]=1

}; readonly -f sst_am_var_set_const
