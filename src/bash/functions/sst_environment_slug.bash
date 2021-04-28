#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_environment_slug() {

  local x

  if (($# != 0)); then
    sst_join "$@" | $FUNCNAME
    return
  fi

  x=$(cat | sst_underscore_slug)
  if [[ "$x" == "" || "$x" == [0-9]* ]]; then
    x=_$x
  fi
  tr a-z A-Z <<<"$x"

}; readonly -f sst_environment_slug
