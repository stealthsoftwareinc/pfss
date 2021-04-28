#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_get_distro_version() {

  # Bash >=4.2: declare -g sst_distro
  # Bash >=4.2: declare -g sst_distro_version

  sst_expect_argument_count $# 0

  if [[ ! "${sst_distro_version+x}" ]]; then
    sst_get_distro >/dev/null
    sst_distro_version=$(sst_${sst_distro}_get_distro_version)
    if [[ ! "$sst_distro_version" ]]; then
      sst_distro_version=unknown
    fi
  fi

  readonly sst_distro_version
  printf '%s\n' $sst_distro_version

}; readonly -f sst_get_distro_version
