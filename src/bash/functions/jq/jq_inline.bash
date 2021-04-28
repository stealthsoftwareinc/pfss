#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

jq_inline() {

  local x

  case $# in
    0)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  x=$1
  readonly x
  shift

  jq "$@" <<EOF
$x
EOF

}; readonly -f jq_inline
