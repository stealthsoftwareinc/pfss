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

sst_barf() {

  # Bash >=4.2: declare -g SST_BARF_STATUS
  # Bash >=4.2: declare -g sst_err_trap_status
  # Bash >=4.2: declare -g sst_root_tmpdir

  declare file
  declare func
  declare i
  declare line
  declare s
  declare status
  declare status_regex
  declare x

  status_regex='^(0|[1-9][0-9]{0,2})$'
  readonly status_regex

  status=1
  if [[ "${SST_BARF_STATUS+x}" ]]; then
    s=$SST_BARF_STATUS
    if [[ "$s" =~ $status_regex ]] && ((s <= 255)); then
      status=$s
    else
      x=$(sst_smart_quote "$s")
      sst_warn "$FUNCNAME: Ignoring invalid SST_BARF_STATUS: $x"
    fi
  elif [[ "${sst_err_trap_status+x}" ]]; then
    status=$sst_err_trap_status
  fi
  readonly status

  if (($# == 0)); then
    set 'Unknown error.'
  fi

  x="$@"
  if [[ ! "${sst_root_tmpdir-}" ]]; then
    # We were called by the prelude before sst_root_tmpdir was ready.
    printf '%s\n' "$0: Error: $x" >&2
  elif mkdir "$sst_root_tmpdir"/sst_barf_leaf &>/dev/null; then
    # We were called by the leaf barfing subshell.
    printf '%s\n' "$0: Error: $x" >&2
  else
    # We were called by a non-leaf barfing subshell.
    printf '%s\n' "  up: Error: $x" >&2
  fi

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
    if [[ "${file##*/}" == sst.bash ]]; then
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

  exit $status

}; readonly -f sst_barf
