#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_centos_get_distro_version() {

  local version

  sst_expect_argument_count $# 0

  version=

  if [[ -f /etc/system-release ]]; then
    version=$(sed -n '
      1 {
        s/^CentOS release 6\..*/6/p
        s/^CentOS Linux release 7\..*/7/p
        s/^CentOS Linux release 8\..*/8/p
        q
      }
    ' /etc/system-release)
  fi

  printf '%s\n' "$version"

}; readonly -f sst_centos_get_distro_version
