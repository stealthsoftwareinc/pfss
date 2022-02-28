#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_json_quote() {

  sst_expect_utf8 "$@" | jq -R -a -s 'rtrimstr("\n")'

}; readonly -f sst_json_quote
