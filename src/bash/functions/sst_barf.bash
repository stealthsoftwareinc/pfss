#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This function may be called by sst_install_utility, so we need to be
# careful to only use utilities that are always available and to write
# the code so that it behaves correctly under errexit suspension.
#

sst_barf() {

  local file
  local func
  local i
  local line
  local x

  # Print the message prefix.
  if [[ ! "${sst_root_tmpdir+x}" ]]; then
    # We were called by the prelude before sst_root_tmpdir was ready.
    printf '%s: error: ' "$0" >&2
  elif mkdir "$sst_root_tmpdir"/sst_barf_leaf &>/dev/null; then
    # We were called by a leaf subshell.
    printf '%s: error: ' "$0" >&2
  else
    # We were called by a non-leaf subshell.
    printf '  up: ' >&2
  fi

  # Print the message.
  if (($# == 0)); then
    printf 'unknown error' >&2
  else
    printf "$@" >&2
  fi
  printf '\n' >&2

  # Print the stack trace.
  i=0
  while x=$(caller $i); do
    i=$((i + 1))

    # Peel off the line number.
    while [[ "$x" == [[:blank:]]* ]]; do x=${x#?}; done
    line=${x%%[[:blank:]]*}
    x=${x#"$line"}

    # Peel off the function name.
    while [[ "$x" == [[:blank:]]* ]]; do x=${x#?}; done
    func=${x%%[[:blank:]]*}
    x=${x#"$func"}

    # Peel off the file name.
    while [[ "$x" == [[:blank:]]* ]]; do x=${x#?}; done
    file=${x%%[[:blank:]]*}
    x=${x#"$file"}

    # Skip the lazy function loading functions and the lazy utility
    # installation functions, but allow any prelude helper functions.
    if [[ "${file##*/}" == prelude.bash ]]; then
      if [[ "$func" != source && "$func" != sst_prelude_* ]]; then
        continue
      fi
    fi

    # Skip the utility wrapper functions.
    if [[ "${file##*/}" == sst_install_utility.bash ]]; then
      if [[ "$func" != sst_install_utility ]]; then
        continue
      fi
    fi

    printf '    at %s(%s:%s)\n' "$func" "$file" "$line" >&2

  done

  # Exit with status sst_barf_status if it's set properly.
  if [[ "${sst_barf_status+x}" ]]; then
    x='^(0|[1-9][0-9]{0,2})$'
    if [[ "$sst_barf_status" =~ $x ]]; then
      if ((sst_barf_status <= 255)); then
        exit $sst_barf_status
      fi
    fi
    printf '  warning: ignoring invalid sst_barf_status value: %s\n' \
           "$sst_barf_status" >&2
  fi

  exit 1

}; readonly -f sst_barf
