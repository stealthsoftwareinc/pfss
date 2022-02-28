#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_kill_all_jobs() {

  declare    j
  declare -a js
  declare    r
  declare    s

  js=$(jobs | awk '{ gsub(/[^0-9]/, "", $1); print $1 }') || return $?
  eval js="($js)"
  readonly js

  r=0
  for j in ${js[@]}; do
    kill "$@" %$j && :
    s=$?
    if ((r == 0 && s != 0)); then
      r=$s
    fi
  done
  readonly r

  return $r

}; readonly -f sst_kill_all_jobs
