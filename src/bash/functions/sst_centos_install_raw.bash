#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_centos_install_raw() {

  local c
  local -A equivs
  local package
  local packages
  local s
  # Bash >=4.2: declare -g sst_centos_install_raw_one_time_setup
  # Bash >=4.2: declare -g -A sst_centos_install_raw_seen
  local -r stderr_cache="$sst_root_tmpdir/$FUNCNAME.stderr_cache"
  local version
  local x
  local xs
  local yum

  equivs['coreutils']+=' coreutils-single'
  equivs['coreutils-single']+=' coreutils'

  readonly equivs

  packages=

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_centos_install_raw_seen[$package]+x}" ]]; then
      continue
    fi
    sst_centos_install_raw_seen[$package]=

    # Skip this package if it's already installed.
    c='yum list installed '$(sst_quote "$package")
    eval "$c" >/dev/null 2>"$stderr_cache" && s=0 || s=$?
    if ((s == 0)); then
      continue
    fi
    if ((s != 1)); then
      cat <"$stderr_cache" >&2 || :
      sst_barf 'command exited with status %s: %s' $s "$c"
    fi

    # Skip this package if an equivalent package is already installed.
    eval xs="(${equivs[$package]-})"
    for x in "${xs[@]-}"; do
      c='yum list installed '$(sst_quote "$x")
      eval "$c" >/dev/null 2>"$stderr_cache" && s=0 || s=$?
      if ((s == 0)); then
        continue 2
      fi
      if ((s != 1)); then
        cat <"$stderr_cache" >&2 || :
        sst_barf 'command exited with status %s: %s' $s "$c"
      fi
    done

    packages+=${packages:+ }$(sst_quote "$package")

  done

  readonly packages

  if [[ ! "${packages:+x}" ]]; then
    return
  fi

  yum='yum -q'
  if [[ ! -t 0 ]]; then
    yum+=' -y'
  fi

  if ! command -v sudo >/dev/null; then
    printf '%s install sudo\n' "$yum" >&2
    su -c "$yum install sudo" >&2
  fi

  readonly yum="sudo $yum"

  if [[ ! "${sst_centos_install_raw_one_time_setup+x}" ]]; then
    sst_echo_eval "$yum install epel-release" >&2
    version=$(sst_get_distro_version)
    case $version in
      (8)
        sst_echo_eval "$yum install 'dnf-command(config-manager)'" >&2
        sst_echo_eval "$yum config-manager --set-enabled PowerTools || $yum config-manager --set-enabled powertools" >&2
      ;;
    esac
    readonly sst_centos_install_raw_one_time_setup=
  fi

  sst_echo_eval "$yum install $packages" >&2

}; readonly -f sst_centos_install_raw
