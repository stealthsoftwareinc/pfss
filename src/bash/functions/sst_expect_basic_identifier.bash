#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_expect_basic_identifier() {

  local arg
  local regex

  regex='[a-zA-Z_][a-zA-Z_0-9]*'
  readonly regex

  for arg; do
    if [[ ! "$arg" =~ $regex ]]; then
      sst_barf 'invalid basic identifier: %s' "$arg"
    fi
  done

}; readonly -f sst_expect_basic_identifier
