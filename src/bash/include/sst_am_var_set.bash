#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_var_set() {

  # Bash >=4.2: declare -g    SST_NDEBUG
  # Bash >=4.2: declare -g -A sst_am_var_const
  # Bash >=4.2: declare -g -A sst_am_var_value

  declare    value
  declare    var

  if ((!SST_NDEBUG)); then
    sst_expect_not_subshell
    if (($# == 0)); then
      sst_expect_argument_count $# 1-
    fi
  fi

  var=$1
  readonly var

  if ((!SST_NDEBUG)); then
    sst_expect_basic_identifier "$var"
  fi

  if [[ ! "${sst_am_var_value[$var]+x}" ]]; then
    sst_am_var_const[$var]=0
  elif ((${sst_am_var_const[$var]})); then
    sst_barf \
      "The Automake variable $var is const and cannot be modified." \
    ;
  fi

  shift

  value="$@"
  value=${value#"${value%%[![:blank:]]*}"}
  value=${value%"${value##*[![:blank:]]}"}
  readonly value

  sst_am_append <<<"$var = $value"
  sst_am_var_value[$var]=$value

}; readonly -f sst_am_var_set
