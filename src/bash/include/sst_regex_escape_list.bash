#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_regex_escape_list() {

  declare    output

  if (($# == 0)); then
    output='($.)'
  else
    output='('$(sst_regex_escape "$1")
    while shift && (($# > 0)); do
      output+='|'$(sst_regex_escape "$1")
    done
    output+=')'
  fi
  readonly output

  printf '%s\n' "$output"

}; readonly -f sst_regex_escape_list
