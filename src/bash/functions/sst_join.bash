#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_join() {

  # Avoid $* so we don't depend on IFS.
  if (($# != 0)); then
    printf '%s' "$1" || return
    shift
    if (($# != 0)); then
      printf ' %s' "$@" || return
    fi
  fi
  echo || return

}; readonly -f sst_join
