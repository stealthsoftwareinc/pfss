#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_json_escape() {

  declare x

  x=$(sst_json_quote "$@")
  readonly x

  printf '%s\n' "${x:1:-1}"

}; readonly -f sst_json_escape
