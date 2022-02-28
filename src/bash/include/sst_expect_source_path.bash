#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_expect_source_path() {

  local e
  local x

  for x; do

    e=

    case $x in

      "")
        e='Source paths must not be empty'
      ;;

      *[!-+./0-9A-Z_a-z]*)
        e='Source paths must only contain [-+./0-9A-Z_a-z] characters'
      ;;

      /*)
        e='Source paths must not begin with a / character'
      ;;

      */)
        e='Source paths must not end with a / character'
      ;;

      *//*)
        e='Source paths must not contain repeated / characters'
      ;;

      . | ./* | */./* | */.)
        e='Source paths must not contain any . components'
      ;;

      .. | ../* | */../* | */..)
        e='Source paths must not contain any .. components'
      ;;

      -* | */-*)
        e='Source path components must not begin with a - character'
      ;;

    esac

    if [[ "$e" != "" ]]; then
      x=$(sst_json_quote "$x")
      sst_barf "$e: $x"
    fi

  done

}; readonly -f sst_expect_source_path
