#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

#
# sst_trap_append <arg> <sigspec>...
#

sst_trap_append() {

  local arg
  local sigspec
  local x

  case $# in
    0 | 1)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  arg=$1
  readonly arg
  shift

  for sigspec; do

    x=$(trap -p -- "$sigspec")

    case $x in
      ?*)
        eval "set -- $x"
        shift $(($# - 2))
        x=$1$'\n'$arg
      ;;
      *)
        x=$arg
      ;;
    esac

    trap -- "$x" "$sigspec"

  done

}; readonly -f sst_trap_append
