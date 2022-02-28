#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_jq_get_string_or_null() {

  sst_expect_argument_count $# 2-3
  jq_expect_string_or_null "$1" "$2"
  if (($# == 2)); then
    jq -r " $1 | select(.)" <"$2"
  else
    sst_expect_basic_identifier "$3"
    eval $3='$(jq -r " $1 | select(.)" <"$2" | sst_csf)'
    sst_csf $3
  fi

}; readonly -f sst_jq_get_string_or_null
