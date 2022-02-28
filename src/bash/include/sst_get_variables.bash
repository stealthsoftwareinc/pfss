#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_get_variables() {

  declare    count
  declare    name
  declare -a names
  declare    regex

  names=$(set -o posix; set)
  names=${names//[] ()[]/}
  eval names="($names)"
  readonly names

  count=0
  for name in ${names[@]+"${names[@]}"}; do
    name=${name%%=*}
    if [[ "$name" == *$'\n'* ]]; then
      continue
    fi
    for regex; do
      if [[ "$name" =~ $regex ]]; then
        printf '%s\n' "$name"
        ((++count))
        continue 2
      fi
    done
  done
  readonly count

  if ((count == 0)); then
    echo
  fi

}; readonly -f sst_get_variables
