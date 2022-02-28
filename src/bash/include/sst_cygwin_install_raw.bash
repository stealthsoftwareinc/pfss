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

sst_cygwin_install_raw() {

  local package
  local packages=
  local q
  # Bash >=4.2: declare -g -A sst_cygwin_install_raw_seen

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_cygwin_install_raw_seen[$package]+x}" ]]; then
      continue
    fi
    sst_cygwin_install_raw_seen[$package]=

    # Skip this package if it's already installed.
    q=$(cygcheck -c "$package" | sed -n '/OK$/p') || sst_barf
    if [[ "${q:+x}" ]]; then
      continue
    fi

    packages+=${packages:+,}$(sst_quote "$package")

  done

  if [[ "${packages:+x}" ]]; then
    sst_barf 'missing packages: %s' "$packages"
  fi

}; readonly -f sst_cygwin_install_raw
