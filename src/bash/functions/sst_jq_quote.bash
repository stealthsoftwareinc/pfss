#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_jq_quote() {

  if (($# == 0)); then
    jq -R -s 'rtrimstr("\n")'
  else
    sst_join "$@" | sst_jq_quote
  fi

}; readonly -f sst_jq_quote
