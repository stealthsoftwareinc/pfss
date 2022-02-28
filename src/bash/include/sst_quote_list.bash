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

sst_quote_list() {

  declare    output

  output=
  while (($# > 0)); do
    output+=${output:+ }
    output+=$(sst_quote "$1") || sst_err_trap "$sst_last_command"
    shift
  done
  readonly output

  printf '%s\n' "$output" || sst_err_trap "$sst_last_command"

}; readonly -f sst_quote_list
