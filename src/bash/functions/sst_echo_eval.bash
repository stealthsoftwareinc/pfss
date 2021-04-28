#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_echo_eval() {

  local x

  if (($# == 0)); then
    x=$(cat | sst_csf)
  else
    x=$(sst_join "$@" | sst_csf)
  fi
  sst_csf x
  printf '%s\n' "$x"
  eval " $x"

}; readonly -f sst_echo_eval
