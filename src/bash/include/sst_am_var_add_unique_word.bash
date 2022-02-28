#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_var_add_unique_word() {

  # Bash >=4.2: declare -g -A sst_am_var_value
  local var
  local word

  sst_expect_argument_count $# 1-

  var=$1
  readonly var
  sst_expect_basic_identifier "$var"

  if [[ ! "${sst_am_var_value[$var]+x}" ]]; then
    sst_am_var_set $var
  fi

  shift
  for word; do
    if [[ "$word" == *[[:blank:]]* ]]; then
      sst_barf
    fi
    if [[    "${sst_am_var_value[$var]}" == "$word "* \
          || "${sst_am_var_value[$var]}" == *" $word "* \
          || "${sst_am_var_value[$var]}" == *" $word" ]]; then
      continue
    fi
    sst_am_append <<<"$var += $word"
    sst_am_var_value[$var]+=${sst_am_var_value[$var]:+ }$word
  done

}; readonly -f sst_am_var_add_unique_word
