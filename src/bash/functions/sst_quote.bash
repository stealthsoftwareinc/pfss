#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_quote() {

  if (($# == 0)); then
    sst_nl | sed "
      s/'/'\\\\''/g
      1 s/^/'/
      \$ s/\$/'/
    " || return
  else
    sst_join "$@" | sst_quote || return
  fi

}; readonly -f sst_quote
