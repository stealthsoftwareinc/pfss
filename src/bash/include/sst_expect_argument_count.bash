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

sst_expect_argument_count() {

  local bad
  bad=1
  local regex_number
  regex_number='^(0|[1-9][0-9]*)$'
  local regex_rangemin
  regex_rangemin='^(0|[1-9][0-9]*)-$'
  local regex_range
  regex_range='^(0|[1-9][0-9]*)-(0|[1-9][0-9]*)$'
  local count
  local arg
  local min
  local max
  local x

  if (($# < 2)); then
    sst_barf 'bad argument count to sst_expect_argument_count itself: %s (expected 2-)' "$#"
  fi
  count=$1

  if [[ ! "$count" =~ $regex_number ]]; then
    sst_barf 'bad argument count syntax: "%s"' "$count"
  fi

  shift
  for arg; do
    if [[ "$arg" =~ $regex_number ]]; then
      if (($arg == $count)); then
        bad=0
      fi
    elif [[ "$arg" =~ $regex_rangemin ]]; then
      if ((${BASH_REMATCH[1]} <= $count)); then
        bad=0
      fi
    elif [[ "$arg" =~ $regex_range ]]; then
      min=${BASH_REMATCH[1]}
      max=${BASH_REMATCH[2]}
      if ((min > max)); then
        sst_barf 'bad argument count predicate syntax: "%s"' $arg
      fi
      if ((min <= count && count <= max)); then
        bad=0
      fi
    else
      sst_barf 'bad argument count syntax: "%s"' "$arg"
    fi
  done

  if ((bad)); then
    x="$@"
    sst_barf "Bad argument count: $count (expected $x)"
  fi

}; readonly -f sst_expect_argument_count
