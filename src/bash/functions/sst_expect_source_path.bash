#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_expect_source_path() {

  local e
  local x

  for x; do

    e=

    case $x in

      "")
        e='source paths must not be empty'
      ;;

      *[!/A-Za-z0-9._-]*)
        e='source paths must only contain /A-Za-z0-9._- characters'
      ;;

      /*)
        e='source paths must not begin with a / character'
      ;;

      */)
        e='source paths must not end with a / character'
      ;;

      *//*)
        e='source paths must not contain repeated / characters'
      ;;

      . | ./* | */./* | */.)
        e='source paths must not contain any . components'
      ;;

      .. | ../* | */../* | */..)
        e='source paths must not contain any .. components'
      ;;

      -* | */-*)
        e='source path components must not begin with a - character'
      ;;

    esac

    if [[ "$e" != "" ]]; then
      x=$(sst_jq_quote "$x")
      sst_barf '%s: %s' "$e" "$x"
    fi

  done

}; readonly -f sst_expect_source_path
