#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

expect_safe_path() {

  local x

  for x; do
    case $x in
      *[!/A-Za-z0-9._-]* | -*)
        sst_barf 'unsafe path: %s' "$x"
      ;;
    esac
  done

}; readonly -f expect_safe_path
