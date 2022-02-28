#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_test_fail() {

  # Bash >=4.2: declare -g -a sst_test_show
  local sst_test_show_a
  local sst_test_show_b
  local sst_test_show_m

  sst_expect_argument_count $# 0

  for sst_test_show_a in "${sst_test_show[@]}"; do
    eval sst_test_show_b=\""$sst_test_show_a"\"
    sst_test_show_b=$(sst_quote "$sst_test_show_b")
    printf '%s\n' "  \"$sst_test_show_a\": $sst_test_show_b"
  done

  exit 1

}; readonly -f sst_test_fail
