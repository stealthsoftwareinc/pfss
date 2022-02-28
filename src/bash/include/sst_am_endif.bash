#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_endif() {

  # Bash >=4.2: declare -g -a sst_am_if_vars

  local a
  local b
  local n

  if [[ ! "${sst_am_if_vars+x}" ]]; then
    sst_barf "Orphan sst_am_endif."
  fi

  sst_expect_argument_count $# 0-1

  n=${#sst_am_if_vars[@]}
  readonly n

  a=
  b=${sst_am_if_vars[n - 1]}
  if [[ "$b" == *' '* ]]; then
    a=${b% *}
    b=${b##* }
  fi
  b=${b#-}

  if (($# == 1)); then
    sst_expect_basic_identifier "${1#!}"
    if [[ "$1" != "$b" ]]; then
      sst_barf "Expected sst_am_endif $b."
    fi
  fi

  while [[ "$b" ]]; do
    sst_am_append <<<"endif $b"
    if [[ "$a" == *' '* ]]; then
      b=${a##* }
      a=${a% *}
    else
      b=$a
      a=
    fi
    b=${b#-}
  done

  unset 'sst_am_if_vars[n - 1]'

}; readonly -f sst_am_endif
