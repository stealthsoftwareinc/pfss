#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_am_var_add_unique_file() {

  local file
  # Bash >=4.2: declare -g -A sst_am_var_value
  local var

  sst_expect_argument_count $# 1-

  var=$1
  readonly var
  sst_expect_basic_identifier "$var"

  if [[ ! "${sst_am_var_value[$var]+x}" ]]; then
    sst_am_var_set $var
  fi

  shift
  for file; do
    sst_expect_source_path "$file"
    if [[    "${sst_am_var_value[$var]}" == "$file "* \
          || "${sst_am_var_value[$var]}" == *" $file "* \
          || "${sst_am_var_value[$var]}" == *" $file" ]]; then
      continue
    fi
    sst_am_append <<<"$var += $file"
    sst_am_var_value[$var]+=${sst_am_var_value[$var]:+ }$file
  done

}; readonly -f sst_am_var_add_unique_file
