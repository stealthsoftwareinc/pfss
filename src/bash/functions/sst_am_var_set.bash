#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_am_var_set() {

  # Bash >=4.2: declare -g -A sst_am_var_value
  local text
  local var

  sst_expect_argument_count $# 1-

  var=$1
  readonly var
  sst_expect_basic_identifier "$var"

  sst_am_append <<<"$var ="
  sst_am_var_value[$var]=

  shift
  for text; do
    sst_am_append <<<"$var += $text"
    sst_am_var_value[$var]+=${sst_am_var_value[$var]:+ }$text
  done

}; readonly -f sst_am_var_set
