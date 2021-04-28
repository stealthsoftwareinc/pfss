#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_underscore_slug() {

  if (($# == 0)); then
    local input
    input=$(cat | sst_csf)
    sst_csf input
    printf '%s\n' "${input//[!0-9A-Za-z]/_}"
  else
    sst_join "$@" | sst_underscore_slug
  fi

}; readonly -f sst_underscore_slug
