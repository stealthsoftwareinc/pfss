#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and run them
# with "command", and we need to explicitly call sst_err_trap on error
# to handle errexit suspension correctly. errexit suspension will occur
# when the user uses idioms such as "foo || s=$?" or "if foo; then" and
# foo triggers our automatic utility installation system. In this case,
# we want to maintain the behavior expected by the user but still barf
# if the installation of foo fails.
#

sst_set_exit() {

  declare entry_status=$?
  readonly entry_status

  # Bash >=4.2: declare -g SST_NDEBUG

  if ((!SST_NDEBUG)); then
    sst_expect_argument_count $# 0-1 || sst_err_trap "$sst_last_command"
    if (($# > 0)); then
      sst_expect_exit_status "$1" || sst_err_trap "$sst_last_command"
    fi
  fi

  return ${1-$entry_status}

}; readonly -f sst_set_exit
