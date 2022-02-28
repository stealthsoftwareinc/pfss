#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

# TODO: After autogen_ac_append is eventually removed, the
# quoted-by-default convention will be gone and we can remove the
# printing of "[" and "]" in this function, as well as removing the
# opening "[" and the trailing "]" in the start and finish functions.

sst_ac_append() {

  if [[ ! "${autogen_ac_start_has_been_called+x}" ]]; then
    sst_barf 'autogen_ac_start has not been called'
  fi

  if [[ "${autogen_ac_finish_has_been_called+x}" ]]; then
    sst_barf 'autogen_ac_finish has been called'
  fi

  sst_expect_argument_count $# 0

  printf ']\n' >>$autogen_ac_file
  cat >>$autogen_ac_file
  printf '[\n' >>$autogen_ac_file

}; readonly -f sst_ac_append
