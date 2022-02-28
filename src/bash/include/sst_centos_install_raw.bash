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

sst_centos_install_raw() {

  # Bash >=4.2: declare -g -A sst_centos_install_raw_seen
  # Bash >=4.2: declare -g sst_centos_install_raw_setup
  # Bash >=4.2: declare -g sst_distro_version

  local cmd
  local -A equivs
  local package
  local package_q
  local -A packages
  local s
  local stderr_cache
  local x
  local xs
  local yum

  equivs['coreutils']+=' coreutils-single'
  equivs['coreutils-single']+=' coreutils'
  readonly equivs

  stderr_cache=$sst_root_tmpdir/$FUNCNAME.stderr_cache
  readonly stderr_cache

  packages=()

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_centos_install_raw_seen[$package]-}" ]]; then
      continue
    fi
    sst_centos_install_raw_seen[$package]=1

    package_q=$(sst_smart_quote "$package") || sst_barf

    # Skip this package if it's already installed.
    cmd="yum list installed -- $package_q"
    eval "$cmd" >/dev/null 2>"$stderr_cache" && s=0 || s=$?
    if ((s == 0)); then
      continue
    fi
    if ((s != 1)); then
      cat <"$stderr_cache" >&2 || :
      sst_barf "Command failed with exit status $s: $cmd"
    fi

    # Skip this package if an equivalent package is already installed.
    eval xs="(${equivs[$package]-})"
    for x in ${xs[@]+"${xs[@]}"}; do
      cmd='yum list installed -- '$(sst_smart_quote "$x") || sst_barf
      eval "$cmd" >/dev/null 2>"$stderr_cache" && s=0 || s=$?
      if ((s == 0)); then
        continue 2
      fi
      if ((s != 1)); then
        cat <"$stderr_cache" >&2 || :
        sst_barf "Command failed with exit status $s: $cmd"
      fi
    done

    packages[$package_q]=1

  done

  if ((${#packages[@]} == 0)); then
    return
  fi

  yum='yum'
  if [[ ! -t 0 ]]; then
    yum="$yum -y"
  fi

  if [[ ! "${sst_centos_install_raw_setup-}" ]]; then
    cmd='type -f -p sudo'
    eval "$cmd" >/dev/null 2>"$stderr_cache" && s=0 || s=$?
    if ((s == 1)); then
      cmd="$yum install sudo"
      printf '%s\n' "su -c '$cmd'" >&2 || sst_barf
      su -c "$cmd" >&2 || sst_barf
    elif ((s != 0)); then
      cat <"$stderr_cache" >&2 || :
      sst_barf "Command failed with exit status $s: $cmd"
    fi
  fi

  yum="sudo $yum"
  readonly yum

  if [[ ! "${sst_centos_install_raw_setup-}" ]]; then
    sst_centos_install_raw_setup=1
    readonly sst_centos_install_raw_setup
    sst_echo_eval "$yum install epel-release" >&2 || sst_barf
    sst_get_distro_version >/dev/null || sst_barf
    if [[ $sst_distro_version == 8 ]]; then
      sst_echo_eval "$yum install dnf-plugins-core" >&2 || sst_barf
      cmd="$yum config-manager --set-enabled powertools"
      cmd+=" || $yum config-manager --set-enabled PowerTools"
      sst_echo_eval "$cmd" >&2 || sst_barf
    fi
  fi

  unset 'packages[dnf-plugins-core]'
  unset 'packages[epel-release]'
  unset 'packages[sudo]'

  readonly packages

  if ((${#packages[@]} == 0)); then
    return
  fi

  sst_push_var IFS ' ' || sst_barf
  sst_echo_eval "$yum install -- ${!packages[*]}" >&2 || sst_barf
  sst_pop_var IFS || sst_barf

}; readonly -f sst_centos_install_raw
