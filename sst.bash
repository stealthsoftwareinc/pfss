#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

#
# This file provides a stable location from which the SST Bash library
# can be included by Bash scripts in this repository. This file should
# always exist in the root of the repository as sst.bash, and the "."
# command below should be adjusted to include the actual sst.bash file
# from its true location in the repository. Scripts can then generally
# use the following inclusion code documented in the "Bash library >
# Including the library" section of the SST manual:
#
#       #! /bin/sh -
#       set -e || exit $?
#       if test -f sst.bash; then
#         . ./sst.bash
#       else
#         . sst.bash
#       fi
#

set -e || exit $?
. src/bash/sst.bash
