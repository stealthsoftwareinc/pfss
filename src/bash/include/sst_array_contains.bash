#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_array_contains() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  if ((!SST_NDEBUG)); then
    if (($# < 1)); then
      sst_expect_argument_count $# 1- || sst_err_trap "$sst_last_command"
    fi
    sst_expect_basic_identifier "$1" || sst_err_trap "$sst_last_command"
  fi

  eval '

    shift

    declare    x'$1'
    declare    y'$1'

    if (($# == 0)); then
      x'$1'=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
      sst_csf x'$1' || sst_err_trap "$sst_last_command"
    else
      x'$1'="$@"
    fi
    readonly x'$1'

    for y'$1' in ${'$1'[@]+"${'$1'[@]}"}; do
      if [[ "$y'$1'" == "$x'$1'" ]]; then
        return 0
      fi
    done

    return 1

  '

}; readonly -f sst_array_contains
