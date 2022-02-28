#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_add_slash_abs_prefix() {

  declare    path

  if (($# == 0)); then
    path=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
    sst_csf path || sst_err_trap "$sst_last_command"
  elif (($# == 1)); then
    path=$1
  else
    sst_expect_argument_count $# 0-1
  fi

  if [[ ! "$path" ]]; then
    sst_barf "A path was empty."
  fi

  if [[ "$path" == *[!/] ]]; then
    path+=/
  fi

  if [[ "$path" ]]; then
    if [[ "$path" == .. || "$path" == */.. ]]; then
      path+=/.
    fi
    if [[ "$path" == */* ]]; then
      path=${path%/*}/
      if [[ "$path" != /* ]]; then
        if [[ "$PWD" == / ]]; then
          path=/$path
        else
          path=$PWD/$path
        fi
      fi
    elif [[ "$PWD" == / ]]; then
      path=/
    else
      path=$PWD/
    fi
  fi

  readonly path

  printf '%s\n' "$path" || sst_err_trap "$sst_last_command"

}; readonly -f sst_add_slash_abs_prefix
