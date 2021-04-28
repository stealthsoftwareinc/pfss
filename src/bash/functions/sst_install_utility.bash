#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_install_utility() {

  # Bash >=4.2: declare -g -A sst_utility_overrides
  # Bash >=4.2: declare -g -A sst_utility_prefixes
  # Bash >=4.2: declare -g -A sst_utility_programs
  # Bash >=4.2: declare -g -A sst_utility_suffixes
  # Bash >=4.2: declare -g -A sst_utility_seen

  local override
  local utility

  if (($# == 0)); then
    return
  fi

  #
  # Allow environment variable overrides. For example, allow the GIT
  # environment variable to override the git utility.
  #

  for utility; do
    if [[ ! "${sst_utility_seen[$utility]+x}" ]]; then
      override=$(sst_environment_slug <<<"$utility")
      eval override=\${$override-}
      if [[ "$override" ]]; then
        sst_utility_overrides[$utility]=$override
        sst_utility_programs[$utility]=
      fi
    fi
  done

  #
  # Install any utilities that haven't already been installed or
  # overridden by environment variables.
  #

  sst_get_distro >/dev/null
  eval sst_${sst_distro}_install_utility '"$@"'

  #
  # Define any wrapper functions that haven't already been defined.
  #
  # It's important that these wrapper functions behave nicely regardless
  # of whether set -e is enabled, as idioms that temporarily suspend the
  # set -e state, like "utility || s = $?" or "if utility; then", should
  # continue to behave as expected.
  #

  for utility; do
    if [[ ! "${sst_utility_seen[$utility]+x}" ]]; then
      eval '
        '"$utility"'() {
          if [[ "${sst_utility_overrides['$utility']+x}" ]]; then
            eval " ${sst_utility_overrides['$utility']}" '\''"$@"'\''
          else
            eval " ${sst_utility_prefixes['$utility']} \
                   command \
                   ${sst_utility_programs['$utility']} \
                   ${sst_utility_suffixes['$utility']}" '\''"$@"'\''
          fi
        }; readonly -f '"$utility"'
      '
      sst_utility_seen[$utility]=x
    fi
  done

}; readonly -f sst_install_utility
