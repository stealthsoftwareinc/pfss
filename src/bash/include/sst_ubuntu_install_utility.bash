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

sst_ubuntu_install_utility() {

  # Bash >=4.2: declare -g -A sst_ubuntu_install_utility_map
  # Bash >=4.2: declare -g sst_ubuntu_install_utility_once

  if ((!${sst_ubuntu_install_utility_once-0})); then
    sst_ubuntu_install_utility_once=1

    #-------------------------------------------------------------------

    sst_ubuntu_install_docker_compose() {
      sst_unix_like_install_docker_compose \
        curl \
        jq \
        sudo \
      || sst_barf
    }; readonly -f sst_ubuntu_install_docker_compose

    #-------------------------------------------------------------------

    sst_ubuntu_install_utility_map=(

      ['awk 16.04']='gawk gawk'
      ['awk 18.04']='gawk gawk'
      ['awk 20.04']='gawk gawk'
      ['c89 16.04']='c89 gcc'
      ['c89 18.04']='c89 gcc'
      ['c89 20.04']='c89 gcc'
      ['c99 16.04']='c99 gcc'
      ['c99 18.04']='c99 gcc'
      ['c99 20.04']='c99 gcc'
      ['cat 16.04']='cat coreutils'
      ['cat 18.04']='cat coreutils'
      ['cat 20.04']='cat coreutils'
      ['cc 16.04']='cc gcc'
      ['cc 18.04']='cc gcc'
      ['cc 20.04']='cc gcc'
      ['curl 16.04']='curl curl'
      ['curl 18.04']='curl curl'
      ['curl 20.04']='curl curl'
      ['docker-compose 16.04']='sst_ubuntu_install_docker_compose'
      ['docker-compose 18.04']='sst_ubuntu_install_docker_compose'
      ['docker-compose 20.04']='sst_ubuntu_install_docker_compose'
      ['gawk 16.04']='gawk gawk'
      ['gawk 18.04']='gawk gawk'
      ['gawk 20.04']='gawk gawk'
      ['git 16.04']='git git'
      ['git 18.04']='git git'
      ['git 20.04']='git git'
      ['gpg1 16.04']='gpg gnupg'
      ['gpg1 18.04']='gpg1 gnupg1'
      ['gpg1 20.04']='gpg1 gnupg1'
      ['gpg2 16.04']='gpg2 gnupg2'
      ['gpg2 18.04']='gpg2 gnupg2'
      ['gpg2 20.04']='gpg2 gnupg2'
      ['jq 16.04']='jq jq'
      ['jq 18.04']='jq jq'
      ['jq 20.04']='jq jq'
      ['make 16.04']='make make'
      ['make 18.04']='make make'
      ['make 20.04']='make make'
      ['mv 16.04']='mv coreutils'
      ['mv 18.04']='mv coreutils'
      ['mv 20.04']='mv coreutils'
      ['sort 16.04']='sort coreutils'
      ['sort 18.04']='sort coreutils'
      ['sort 20.04']='sort coreutils'
      ['ssh 16.04']='ssh openssh-client'
      ['ssh 18.04']='ssh openssh-client'
      ['ssh 20.04']='ssh openssh-client'
      ['ssh-keygen 16.04']='ssh-keygen openssh-client'
      ['ssh-keygen 18.04']='ssh-keygen openssh-client'
      ['ssh-keygen 20.04']='ssh-keygen openssh-client'
      ['sshpass 16.04']='sshpass sshpass'
      ['sshpass 18.04']='sshpass sshpass'
      ['sshpass 20.04']='sshpass sshpass'
      ['sudo 16.04']='sudo sudo'
      ['sudo 18.04']='sudo sudo'
      ['sudo 20.04']='sudo sudo'
      ['tar 16.04']='tar tar'
      ['tar 18.04']='tar tar'
      ['tar 20.04']='tar tar'
      ['wget 16.04']='wget wget'
      ['wget 18.04']='wget wget'
      ['wget 20.04']='wget wget'

    )

    #-------------------------------------------------------------------

  fi

  sst_install_utility_from_map \
    sst_ubuntu_install_utility_map \
    "$@" \
  || sst_barf

}; readonly -f sst_ubuntu_install_utility
