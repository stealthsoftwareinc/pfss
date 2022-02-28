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

build-pages() {

  local dist_archive

  dist_archive=$(sst_find_dist_archive)
  readonly dist_archive

  mkdir "$sst_tmpdir"/x
  sst_pushd "$sst_tmpdir"/x
  sst_extract_archive "$sst_rundir"/"$dist_archive"
  sst_popd

  # TODO: The default should be to copy docs/ from the distribution
  #       repository, as this is the strategy for GitHub, but we should
  #       let an arbitrary directory or archive within the distribution
  #       archive to be specified.

  mkdir "$sst_tmpdir"/y
  sst_pushd "$sst_tmpdir"/y
  cp -R -L "$sst_tmpdir"/x/*/docs .
  sst_popd

  # TODO: Use sst_mkdir_p_new_only after it's implemented.
  mkdir public
  rmdir public
  mv -f "$sst_tmpdir"/y/* public

}; readonly -f build-pages

build-pages "$@"
