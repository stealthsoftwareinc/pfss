#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

autogen_am_finish() {

  case ${autogen_am_start_has_been_called+x} in
    "")
      sst_barf 'autogen_am_start has not been called'
    ;;
  esac

  case ${autogen_am_finish_has_been_called+x} in
    ?*)
      sst_barf 'autogen_am_finish has already been called'
    ;;
  esac
  autogen_am_finish_has_been_called=x
  readonly autogen_am_finish_has_been_called

  case $# in
    0)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

}; readonly -f autogen_am_finish
