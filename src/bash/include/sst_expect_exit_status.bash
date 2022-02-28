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

sst_expect_exit_status() {

  declare regex
  declare x

  regex='^(0|[1-9][0-9]{0,2})$'
  readonly regex

  for x; do
    if [[ ! "$x" =~ $regex ]] || ((x > 255)); then
      x=$(sst_smart_quote "$x") || sst_err_trap "$sst_last_command"
      sst_barf \
        "Invalid exit status: $x." \
        "An exit status must match /$regex/ and be less than 256." \
      ;
    fi
  done

}; readonly -f sst_expect_exit_status
