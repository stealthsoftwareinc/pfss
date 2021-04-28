#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
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
    q=$(cygcheck -c "$package" | sed -n '/OK$/p') || return
    if [[ "${q:+x}" ]]; then
      continue
    fi

    packages+=${packages:+,}$(sst_quote "$package")

  done

  if [[ "${packages:+x}" ]]; then
    sst_barf 'missing packages: %s' "$packages"
  fi

}; readonly -f sst_cygwin_install_raw
