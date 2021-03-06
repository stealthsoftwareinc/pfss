#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

jq_expect_types_or_null() {

  case $# in
    3)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  case $3 in
    array | boolean | null | number | object | string)
    ;;
    *)
      sst_barf '$3: invalid type: %s' "$3"
    ;;
  esac

  sst_jq_expect "
    $1 | (
      (type == \"null\") or
      (type == \"array\" and (map(type == \"$3\") | all))
    )
  " '%s: %s: expected array of %ss or null' "$2" "$1" $3 <"$2"

}; readonly -f jq_expect_types_or_null
