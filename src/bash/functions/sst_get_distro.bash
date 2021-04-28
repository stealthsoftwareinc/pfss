#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#
# Note that the uname utility isn't very useful for determining the
# distro because it may return information about the host system when
# run in a container. This usually happens because containers usually
# use the host kernel, not their own kernels.
#

sst_get_distro() {

  # Bash >=4.2: declare -g sst_distro
  local x

  sst_expect_argument_count $# 0

  while [[ ! "${sst_distro+x}" ]]; do

    if [[ -f /etc/os-release ]]; then

      x=$(
        sed -n '
          /^ID=/ {
            s/^ID=//
            s/"//g
            p
            q
          }
        ' /etc/os-release
      ) || return

      case $x in
        ( $'\n' \
        | alpine \
        | arch \
        | centos \
        | debian \
        | fedora \
        | rhel \
        | ubuntu \
        )
          sst_distro=$x
          break
        ;;
      esac

    fi

    if command -v cygcheck >/dev/null; then
      sst_distro=cygwin
      break
    fi

    if command -v cygpath >/dev/null; then
      if command -v pacman >/dev/null; then
        sst_distro=msys2
        break
      fi
    fi

    if command -v sw_vers >/dev/null; then
      if command -v brew >/dev/null; then
        sst_distro=homebrew
        break
      fi
      if command -v port >/dev/null; then
        sst_distro=macports
        break
      fi
    fi

    sst_distro=unknown
    break

  done

  readonly sst_distro
  printf '%s\n' $sst_distro

}; readonly -f sst_get_distro
