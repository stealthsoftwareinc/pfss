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

sst_get_distro_version() {

  # Bash >=4.2: declare -g sst_distro
  # Bash >=4.2: declare -g sst_distro_version

  sst_expect_argument_count $# 0

  if [[ ! "${sst_distro_version-}" ]]; then
    sst_get_distro >/dev/null || sst_barf
    sst_distro_version=$(sst_${sst_distro}_get_distro_version) || sst_barf
    if [[ ! "$sst_distro_version" ]]; then
      sst_distro_version=unknown
    fi
  fi

  readonly sst_distro_version

  printf '%s\n' "$sst_distro_version"

}; readonly -f sst_get_distro_version
