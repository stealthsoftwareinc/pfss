#! /bin/sh -
#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#
# This script uses the SST Bash library to bootstrap from /bin/sh into
# bash. See the Bash library > Bootstrapping section of the SST manual
# for more information.
#

#-----------------------------------------------------------------------
# Include the SST Bash library
#-----------------------------------------------------------------------
#
# Include the first sst.bash file found by searching up the directory
# tree starting from the location of this script file. If no such file
# is found, fall back to including plain sst.bash with PATH resolution.
# Use SST_BASH_PRELUDE_PATH to cache the location of the sst.bash file
# to prevent a redundant search from occurring during the bootstrap.
#

case ${SST_BASH_PRELUDE_PATH-} in '')
  case $0 in /*)
    x=$0
  ;; *)
    x=./$0
  esac
  r='\(.*/\)'
  x=`expr "x${x?}" : "x${r?}"`. || exit $?
  SST_BASH_PRELUDE_PATH=`
    cd "${x?}" || exit $?
    while :; do
      if test -f sst.bash; then
        case ${PWD?} in *[!/]*)
          printf '%s\n' "${PWD?}"/sst.bash || exit $?
        ;; *)
          printf '%s\n' "${PWD?}"sst.bash || exit $?
        esac
        exit
      fi
      case ${PWD?} in *[!/]*)
        cd ..
      ;; *)
        printf '%s\n' sst.bash || exit $?
        exit
      esac
    done
  ` || exit $?
  export SST_BASH_PRELUDE_PATH
esac
set -e || exit $?
. "${SST_BASH_PRELUDE_PATH?}"
unset SST_BASH_PRELUDE_PATH

#-----------------------------------------------------------------------

dist_archive=$rundir/$(sst_find_dist_archive)
readonly dist_archive

cd "$tmpdir"
sst_extract_archive "$dist_archive"
cd *
install/on-ubuntu
make V=1 VERBOSE=1 distcheck
