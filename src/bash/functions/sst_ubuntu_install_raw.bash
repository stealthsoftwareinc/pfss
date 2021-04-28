#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_ubuntu_install_raw() {

  local apt_get
  local c
  local package
  local packages
  local q
  local s
  # Bash >=4.2: declare -g sst_ubuntu_install_raw_one_time_setup
  # Bash >=4.2: declare -g -A sst_ubuntu_install_raw_seen
  local -r stderr_cache="$sst_root_tmpdir/$FUNCNAME.stderr_cache"

  packages=

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_ubuntu_install_raw_seen[$package]+x}" ]]; then
      continue
    fi
    sst_ubuntu_install_raw_seen[$package]=

    # Skip this package if it's already installed.
    c='dpkg-query -W -f '\''${db:Status-Status}'\'' '
    c+=$(sst_quote "$package")
    q=$(eval "$c" 2>"$stderr_cache") && s=0 || s=$?
    if [[ $s == 0 && "$q" == installed ]]; then
      continue
    fi
    if [[ $s != 0 && $s != 1 ]]; then
      cat <"$stderr_cache" >&2 || :
      sst_barf 'command exited with status %s: %s' $s "$c"
    fi

    packages+=${packages:+ }$(sst_quote "$package")

  done

  readonly packages

  if [[ ! "${packages:+x}" ]]; then
    return
  fi

  apt_get='apt-get -q'
  if [[ ! -t 0 ]]; then
    apt_get='DEBIAN_FRONTEND=noninteractive '$apt_get
    apt_get+=' -y'
  fi

  if ! command -v sudo >/dev/null; then
    if [[ ! "${sst_ubuntu_install_raw_one_time_setup+x}" ]]; then
      printf '%s update && %s install sudo\n' "$apt_get" "$apt_get" >&2
      su -c "$apt_get update && $apt_get install sudo" >&2
      readonly sst_ubuntu_install_raw_one_time_setup=x
    else
      printf '%s install sudo\n' "$apt_get" >&2
      su -c "$apt_get install sudo" >&2
    fi
  fi

  readonly apt_get="sudo $apt_get"

  if [[ ! "${sst_ubuntu_install_raw_one_time_setup+x}" ]]; then
    sst_echo_eval "$apt_get update" >&2
    readonly sst_ubuntu_install_raw_one_time_setup=
  fi

  sst_echo_eval "$apt_get install $packages" >&2

}; readonly -f sst_ubuntu_install_raw
