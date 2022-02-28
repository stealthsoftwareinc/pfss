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

sst_echo_eval() {

  local x

  if (($# == 0)); then
    x=$(cat | sst_csf) || sst_barf
    sst_csf x || sst_barf
    set -- "$x"
  fi
  sst_join "$@" || sst_barf
  eval " $@" || sst_barf

}; readonly -f sst_echo_eval
