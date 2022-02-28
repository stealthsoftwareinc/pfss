#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_am_append() {

  if [[ ! "${autogen_am_start_has_been_called+x}" ]]; then
    sst_barf 'autogen_am_start has not been called'
  fi

  if [[ "${autogen_am_finish_has_been_called+x}" ]]; then
    sst_barf 'autogen_am_finish has been called'
  fi

  sst_expect_argument_count $# 0

  cat >>$autogen_am_file

}; readonly -f sst_am_append
