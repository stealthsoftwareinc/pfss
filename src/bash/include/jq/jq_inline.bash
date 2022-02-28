#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
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
