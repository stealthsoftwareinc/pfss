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

function main {

  local -a build_depends
  local kv_ASCIIDOCTOR
  local parse_options
  local tag
  local tags

  build_depends=()

  parse_options=1
  set x "$@"
  while shift && (($# > 0)); do

    if ((parse_options)); then

      #-----------------------------------------------------------------
      # Options terminator
      #-----------------------------------------------------------------

      sst_parse_opt -- forbidden : "$@"
      eval "$pop"
      if ((got)); then
        parse_options=
        continue
      fi

      #-----------------------------------------------------------------
      # --build-depends
      #-----------------------------------------------------------------

      sst_parse_opt --build-depends : "$@"
      eval "$pop"
      if ((got)); then
        build_depends+=("$arg")
        continue
      fi

      #-----------------------------------------------------------------
      # Unknown options
      #-----------------------------------------------------------------

      sst_unknown_opt "$1"

      #-----------------------------------------------------------------

    fi

    sst_barf "operands are forbidden"

  done
  readonly parse_options

  readonly build_depends

  sst_ubuntu_install_raw "${build_depends[@]}"

  #---------------------------------------------------------------------
  #
  # If we're building in a GitLab CI pipeline that wasn't started by a
  # tag, we don't want to produce a tag-like version number, as this
  # tends to cause problems in downstream CI jobs. To ensure this
  # doesn't happen, we delete any tags pointing at HEAD.
  #
  # This situation usually happens when a branch update and a tag that
  # points at the new branch tip are pushed at the same time. The push
  # will start two pipelines: one for the branch, and one for the tag.
  # When the branch pipeline eventually starts its first job, the tag
  # will already be there, causing a version number like 0.19.0 to be
  # generated instead of the desired 0.18.2-286+gb74639261.
  #

  if [[ "${GITLAB_CI-}" && ! "${CI_COMMIT_TAG-}" ]]; then
    tags=$(git tag --points-at HEAD)
    for tag in $tags; do
      git tag -d -- "$tag"
    done
  fi

  #---------------------------------------------------------------------

  ./autogen

  kv_ASCIIDOCTOR=
  if [[ -x build-aux/adock ]]; then
    kv_ASCIIDOCTOR='ASCIIDOCTOR="build-aux/adock"'
  fi

  eval ./configure \
    "$kv_ASCIIDOCTOR" \
  ;

  make dist

}; readonly -f main

main "$@"
