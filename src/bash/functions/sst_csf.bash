#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_csf() {

  if (($# == 0)); then
    cat || return
    echo x || return
  else
    while (($# != 0)); do
      sst_expect_basic_identifier "$1" || return
      eval "$1=\${$1%x}"
      eval "$1=\${$1%\$'\\n'}"
      shift
    done
  fi

}; readonly -f sst_csf
