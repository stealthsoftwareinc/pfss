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

sst_centos_install_utility() {

  # Bash >=4.2: declare -g -A sst_centos_install_utility_map
  # Bash >=4.2: declare -g sst_centos_install_utility_once

  if ((!${sst_centos_install_utility_once-0})); then
    sst_centos_install_utility_once=1

    #-------------------------------------------------------------------

    sst_centos_install_docker_compose() {
      sst_unix_like_install_docker_compose jq sudo wget
    }; readonly -f sst_centos_install_docker_compose

    #-------------------------------------------------------------------

    sst_centos_install_utility_map=(

      ['awk 6']='gawk gawk'
      ['awk 7']='gawk gawk'
      ['awk 8']='gawk gawk'
      ['c89 6']='c89 gcc'
      ['c89 7']='c89 gcc'
      ['c89 8']='c89 gcc'
      ['c99 6']='c99 gcc'
      ['c99 7']='c99 gcc'
      ['c99 8']='c99 gcc'
      ['cat 6']='cat coreutils'
      ['cat 7']='cat coreutils'
      ['cat 8']='cat coreutils'
      ['cc 6']='cc gcc'
      ['cc 7']='cc gcc'
      ['cc 8']='cc gcc'
      ['docker-compose 6']='sst_centos_install_docker_compose'
      ['docker-compose 7']='sst_centos_install_docker_compose'
      ['docker-compose 8']='sst_centos_install_docker_compose'
      ['gawk 6']='gawk gawk'
      ['gawk 7']='gawk gawk'
      ['gawk 8']='gawk gawk'
      ['git 6']='git git'
      ['git 7']='git git'
      ['git 8']='git git-core'
      ['gpg2 6']='gpg2 gnupg2'
      ['gpg2 7']='gpg2 gnupg2'
      ['gpg2 8']='gpg2 gnupg2'
      ['jq 6']='jq jq'
      ['jq 7']='jq jq'
      ['jq 8']='jq jq'
      ['make 6']='make make'
      ['make 7']='make make'
      ['make 8']='make make'
      ['mv 6']='mv coreutils'
      ['mv 7']='mv coreutils'
      ['mv 8']='mv coreutils'
      ['sort 6']='sort coreutils'
      ['sort 7']='sort coreutils'
      ['sort 8']='sort coreutils'
      ['ssh 6']='ssh openssh-clients'
      ['ssh 7']='ssh openssh-clients'
      ['ssh 8']='ssh openssh-clients'
      ['ssh-keygen 6']='ssh-keygen openssh'
      ['ssh-keygen 7']='ssh-keygen openssh'
      ['ssh-keygen 8']='ssh-keygen openssh'
      ['sshpass 6']='sshpass sshpass'
      ['sshpass 7']='sshpass sshpass'
      ['sshpass 8']='sshpass sshpass'
      ['sudo 6']='sudo sudo'
      ['sudo 7']='sudo sudo'
      ['sudo 8']='sudo sudo'
      ['tar 6']='tar tar'
      ['tar 7']='tar tar'
      ['tar 8']='tar tar'
      ['wget 6']='wget wget'
      ['wget 7']='wget wget'
      ['wget 8']='wget wget'

    )

    #-------------------------------------------------------------------

  fi

  sst_install_utility_from_map \
    sst_centos_install_utility_map \
    "$@" \
  || sst_barf

}; readonly -f sst_centos_install_utility
