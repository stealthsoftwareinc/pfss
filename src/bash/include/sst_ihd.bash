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

sst_ihd() {

  local adjust_n
  local n

  sst_expect_argument_count $# 0-1

  if (($# == 1)); then
    n=$1
    # TODO: sst_expect_integer "$n"
    if ((n < 0)); then
      adjust_n=1
    else
      adjust_n=0
    fi
  else
    n=0
    adjust_n=1
  fi
  readonly n
  readonly adjust_n

  awk -v n=$n -v adjust_n=$adjust_n '
    {
      if ($0 != "") {
        if (!have_indent) {
          if (adjust_n) {
            x = $0
            sub(/[^ ].*/, "", x)
            n += length(x)
          }
          for (i = 0; i < n; ++i) {
            indent = indent " "
          }
          have_indent = 1
        }
        if (substr($0, 1, n) == indent) {
          $0 = substr($0, n + 1)
        }
      }
      print
    }
  ' || sst_barf

}; readonly -f sst_ihd
