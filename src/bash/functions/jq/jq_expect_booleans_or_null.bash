#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

jq_expect_booleans_or_null() {

  case $# in
    2)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  jq_expect_types_or_null "$1" "$2" boolean

}; readonly -f jq_expect_booleans_or_null
