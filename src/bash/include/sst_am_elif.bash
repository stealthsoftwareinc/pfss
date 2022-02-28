#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_elif() {

  # Bash >=4.2: declare -g -a sst_am_if_vars

  local a
  local b
  local n

  if [[ ! "${sst_am_if_vars+x}" ]]; then
    sst_barf "Orphan sst_am_elif."
  fi

  sst_expect_argument_count $# 1-2

  sst_expect_basic_identifier "${1#!}"

  n=${#sst_am_if_vars[@]}
  readonly n

  a=
  b=${sst_am_if_vars[n - 1]}
  if [[ "$b" == *' '* ]]; then
    a=${b% *}
    b=${b##* }
  fi
  if [[ "$b" == -* ]]; then
    sst_barf "Orphan sst_am_elif."
  fi
  b=!$b
  b=${b#!!}
  readonly a
  readonly b

  if (($# == 2)); then
    sst_expect_basic_identifier "${2#!}"
    if [[ "$2" != "$b" ]]; then
      sst_barf "Expected sst_am_elif $1 $b."
    fi
  fi

  sst_am_append <<<"else $b"
  sst_am_append <<<"if $1"

  if [[ "$a" ]]; then
    sst_am_if_vars[n - 1]="$a -$b $1"
  else
    sst_am_if_vars[n - 1]="-$b $1"
  fi

}; readonly -f sst_am_elif
