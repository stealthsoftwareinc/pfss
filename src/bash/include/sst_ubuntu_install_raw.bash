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

sst_ubuntu_install_raw() {

  # Bash >=4.2: declare -g -A sst_ubuntu_install_raw_seen
  # Bash >=4.2: declare -g sst_ubuntu_install_raw_setup

  local apt_get
  local cmd
  local package
  local package_q
  local -A packages
  local r
  local s
  local stderr_cache

  stderr_cache=$sst_root_tmpdir/$FUNCNAME.stderr_cache
  readonly stderr_cache

  packages=()

  for package; do

    # Skip this package if we've already seen it.
    if [[ "${sst_ubuntu_install_raw_seen[$package]-}" ]]; then
      continue
    fi
    sst_ubuntu_install_raw_seen[$package]=1

    package_q=$(sst_smart_quote "$package") || sst_barf

    # Skip this package if it's already installed.
    cmd="dpkg-query -W -f '\${db:Status-Status}' -- $package_q"
    r=$(eval "$cmd" 2>"$stderr_cache") && s=0 || s=$?
    if [[ $s == 0 && "$r" == installed ]]; then
      continue
    fi
    if [[ $s != 0 && $s != 1 ]]; then
      cat <"$stderr_cache" >&2 || :
      sst_barf "Command failed with exit status $s: $cmd"
    fi

    packages[$package_q]=1

  done

  if ((${#packages[@]} == 0)); then
    return
  fi

  apt_get='apt-get -q'
  if [[ ! -t 0 ]]; then
    apt_get="DEBIAN_FRONTEND=noninteractive $apt_get -y"
  fi

  if [[ ! "${sst_ubuntu_install_raw_setup-}" ]]; then
    sst_ubuntu_install_raw_setup=1
    readonly sst_ubuntu_install_raw_setup
    cmd='type -f -p sudo >/dev/null'
    eval "$cmd" 2>"$stderr_cache" && s=0 || s=$?
    if ((s == 0)); then
      sst_echo_eval "sudo $apt_get update" >&2 || sst_barf
    elif ((s == 1)); then
      cmd="$apt_get update && $apt_get install sudo"
      printf '%s\n' "su -c '$cmd'" >&2 || sst_barf
      su -c "$cmd" >&2 || sst_barf
    else
      cat <"$stderr_cache" >&2 || :
      sst_barf "Command failed with exit status $s: $cmd"
    fi
  fi

  unset 'packages[sudo]'

  readonly packages

  if ((${#packages[@]} == 0)); then
    return
  fi

  apt_get="sudo $apt_get"
  readonly apt_get

  sst_push_var IFS ' ' || sst_barf
  sst_echo_eval "$apt_get install -- ${!packages[*]}" >&2 || sst_barf
  sst_pop_var IFS || sst_barf

}; readonly -f sst_ubuntu_install_raw
