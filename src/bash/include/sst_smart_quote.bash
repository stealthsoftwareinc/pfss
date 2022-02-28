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

sst_smart_quote() {

  local ok
  local x

  ok='^'\''[+./0-9:=A-Z_a-z-]+'\''$'
  readonly ok

  x=$(sst_quote "$@") || sst_barf
  if [[ "$x" =~ $ok ]]; then
    x=${x#?}
    x=${x%?}
  fi
  printf '%s\n' "$x" || sst_barf

}; readonly -f sst_smart_quote
