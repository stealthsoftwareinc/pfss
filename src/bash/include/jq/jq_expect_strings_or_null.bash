#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

jq_expect_strings_or_null() {

  case $# in
    2)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  jq_expect_types_or_null "$1" "$2" string

}; readonly -f jq_expect_strings_or_null
