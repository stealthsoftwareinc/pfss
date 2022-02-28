#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_copyright_notice() {

  declare    comment1
  declare    comment2
  declare    comment3
  declare    default_prose
  declare    notice
  declare    prose

  default_prose='
Copyright (C) Stealth Software Technologies, Inc.

For the complete copyright information, please see the
associated README file.
'
  default_prose=${default_prose#?}
  readonly default_prose

  case $# in 1)
    comment1=$1
    comment2=$1
    comment3=$1
    prose=$default_prose
  ;; 2)
    comment1=$1
    comment2=$1
    comment3=$1
    prose=$2
  ;; 3)
    comment1=$1
    comment2=$2
    comment3=$3
    prose=$default_prose
  ;; 4)
    comment1=$1
    comment2=$2
    comment3=$3
    prose=$4
  ;; *)
    sst_expect_argument_count $# 1-4
  esac

  readonly comment1
  readonly comment2
  readonly comment3

  if [[ "$prose" == - ]]; then
    prose=$(cat | sst_csf) || sst_err_trap "$sst_last_command"
    sst_csf prose || sst_err_trap "$sst_last_command"
  elif [[ "$prose" == /* || "$prose" == ./* ]]; then
    prose=$(cat "$prose" | sst_csf) || sst_err_trap "$sst_last_command"
    sst_csf prose || sst_err_trap "$sst_last_command"
  else
    prose=${prose%$'\n'}
  fi

  readonly prose

  notice=$comment1
  notice+=$'\n'
  notice+="$comment2 "
  notice+=${prose//$'\n'/$'\n'"$comment2 "}
  notice+=$'\n'
  notice+=$comment3

  readonly notice

  printf '%s\n' "$notice"

}; readonly -f sst_copyright_notice
