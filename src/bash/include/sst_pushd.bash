#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_pushd() {

  pushd "$@" >/dev/null

}; readonly -f sst_pushd
