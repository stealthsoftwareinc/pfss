#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_array_from_zterm_helper() {

  od -A n -t o1 -v \
  | tr ' ' '\\' \
  | sed '
      1 s/^/$'\''/
      $ s/\\000$//
      $ s/$/'\''n/
      s/\\000/'\''n$'\''/g
    ' \
  | tr -d '\n' \
  | tr n '\n' \
  ;

  printf '%s\n' ")"

}; readonly -f sst_array_from_zterm_helper

sst_array_from_zterm() {

  # Bash >=4.2: declare -g    SST_NDEBUG

  if ((!SST_NDEBUG)); then
    if (($# < 1)); then
      sst_expect_argument_count $# 1-
    fi
    sst_expect_basic_identifier "$1"
  fi

  printf '%s\n' "$1=(" >"$sst_root_tmpdir/$FUNCNAME.$$.x"

  if (($# == 1)); then
    sst_array_from_zterm_helper >>"$sst_root_tmpdir/$FUNCNAME.$$.x"
  else
    shift
    "$@" | sst_array_from_zterm_helper >>"$sst_root_tmpdir/$FUNCNAME.$$.x"
  fi

  . "$sst_root_tmpdir/$FUNCNAME.$$.x"

}; readonly -f sst_array_from_zterm
