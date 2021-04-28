#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

jq_expect_object() {

  case $# in
    2)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  jq_expect_type "$1" "$2" object

}; readonly -f jq_expect_object
