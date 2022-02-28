#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_popd() {

  popd "$@" >/dev/null

}; readonly -f sst_popd
