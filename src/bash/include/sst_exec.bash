#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_exec() {

  ((--SST_PRELUDE_DEPTH, 1))

  exec "$@" || sst_err_trap "$sst_last_command"

}; readonly -f sst_exec
