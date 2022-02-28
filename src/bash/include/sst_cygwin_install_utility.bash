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

sst_cygwin_install_utility() {

  # Bash >=4.2: declare -g -A sst_cygwin_install_utility_map
  # Bash >=4.2: declare -g sst_cygwin_install_utility_once

  if ((!${sst_cygwin_install_utility_once-0})); then

    sst_cygwin_install_utility_once=1

    sst_cygwin_install_utility_map=(

      ['awk none']='gawk gawk'
      ['c89 none']='c89 gcc-core'
      ['c99 none']='c99 gcc-core'
      ['cat none']='cat coreutils'
      ['cc none']='cc gcc-core'
      ['gawk none']='gawk gawk'
      ['git none']='git git'
      ['gpg1 none']='gpg gnupg'
      ['gpg2 none']='gpg2 gnupg2'
      ['jq none']='jq jq'
      ['make none']='make make'
      ['mv none']='mv coreutils'
      ['sort none']='sort coreutils'
      ['ssh none']='ssh openssh'
      ['ssh-keygen none']='ssh-keygen openssh'
      ['sshpass none']='sshpass sshpass'
      ['tar none']='tar tar'

    )

  fi

  sst_install_utility_from_map \
    sst_cygwin_install_utility_map \
    "$@" \
  || sst_barf

}; readonly -f sst_cygwin_install_utility
