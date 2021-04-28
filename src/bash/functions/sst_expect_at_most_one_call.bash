#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_expect_at_most_one_call() {

  local i
  local v
  local x

  # We need to look upward by two during the lazily loaded call.
  if [[ "${FUNCNAME[1]}" == "$FUNCNAME" ]]; then
    i=2
  else
    i=1
  fi
  readonly i

  if [[ "${FUNCNAME[i]}" == main ]]; then
    sst_barf '%s must only be called within a function' "$FUNCNAME"
  fi

  sst_expect_argument_count $# 0-1

  if (($# < 1)); then
    v=$(sst_underscore_slug "${FUNCNAME[i]}")_has_already_been_called
  else
    sst_expect_basic_identifier "$1"
    v=$1
  fi
  readonly v

  eval "x=\${$v+x}"
  if [[ "$x" ]]; then
    sst_barf '%s has already been called' "${FUNCNAME[i]}"
  fi

  eval "readonly $v=x"

}; readonly -f sst_expect_at_most_one_call
