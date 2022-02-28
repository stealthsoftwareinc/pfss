#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

# TODO: This should be able to just use ((BASH_SUBSHELL > 0)).

sst_expect_not_subshell() {

  local f

  sst_expect_argument_count $# 0-1

  if [[ "$BASHPID" != "$$" ]]; then

    if [[ $# == 1 ]]; then
      f=$1
    elif [[ "${FUNCNAME[1]}" == "${FUNCNAME[0]}" ]]; then
      f=${FUNCNAME[2]}
    else
      f=${FUNCNAME[1]}
    fi
    readonly f

    if [[ "$f" == - ]]; then
      sst_barf '%s' "expected no subshell"
    else
      sst_barf '%s' "$f must not be called in a subshell"
    fi

  fi

}; readonly -f sst_expect_not_subshell
