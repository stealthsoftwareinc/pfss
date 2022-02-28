#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_var_add_unique_file() {

  local path
  # Bash >=4.2: declare -g -A sst_am_var_value
  # Bash >=4.2: declare -g -A sst_am_var_value_files
  local var

  sst_expect_argument_count $# 1-

  var=$1
  readonly var
  sst_expect_basic_identifier "$var"

  if [[ "${sst_am_var_value[$var]-}" == "" ]]; then
    sst_am_var_set $var
  fi

  shift
  for path; do
    sst_expect_source_path "$path"
    sst_expect_file $path
    if [[ "${sst_am_var_value_files[$var]= }" == *" $path "* ]]; then
      continue
    fi
    sst_am_var_value_files[$var]+="$path "
    sst_am_var_value[$var]+="$path "
    sst_am_append <<<"$var += $path"
  done

}; readonly -f sst_am_var_add_unique_file
