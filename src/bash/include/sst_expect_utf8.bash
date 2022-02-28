#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_utf8() {

  if (($# == 0)); then
    iconv -f UTF-8 -t UTF-8
  else
    sst_join "$@" | sst_expect_utf8
  fi

}; readonly -f sst_expect_utf8
