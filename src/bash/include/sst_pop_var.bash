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

sst_pop_var() {

  sst_expect_argument_count $# 1

  sst_expect_basic_identifier "$1"

  eval '
    # Bash >=4.2: declare -g sst_var_depth_'$1'
    local -r sst_d=$((${sst_var_depth_'$1'-0} - 1))
    sst_var_depth_'$1'=$sst_d

    if ((sst_d < 0)); then
      sst_barf \
        "sst_pop_var $1 was called without" \
        "a corresponding call to sst_push_var $1." \
      ;
    fi

    # Bash >=4.2: declare -g sst_var_unset_${sst_d}_'$1'
    # Bash >=4.2: declare -g sst_var_value_${sst_d}_'$1'

    local sst_u
    eval sst_u=\$sst_var_unset_${sst_d}_'$1'
    readonly sst_u

    if ((sst_u)); then
      unset '$1'
    else
      eval '$1'=\$sst_var_value_${sst_d}_'$1'
    fi
  '

}; readonly -f sst_pop_var
