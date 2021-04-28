#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_ihd() {

  local adjust_n
  local n

  sst_expect_argument_count $# 0-1

  if (($# == 1)); then
    n=$1
    # TODO: sst_expect_integer "$n"
    if ((n < 0)); then
      adjust_n=1
    else
      adjust_n=0
    fi
  else
    n=0
    adjust_n=1
  fi
  readonly n
  readonly adjust_n

  awk -v n=$n -v adjust_n=$adjust_n '
    {
      if ($0 != "") {
        if (!have_indent) {
          if (adjust_n) {
            x = $0
            sub(/[^ ].*/, "", x)
            n += length(x)
          }
          for (i = 0; i < n; ++i) {
            indent = indent " "
          }
          have_indent = 1
        }
        if (substr($0, 1, n) == indent) {
          $0 = substr($0, n + 1)
        }
      }
      print
    }
  ' || return

}; readonly -f sst_ihd
