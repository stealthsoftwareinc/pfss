#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_regex_escape() {

  declare    c
  declare    i
  declare    input
  declare    output

  if (($# == 0)); then
    input=$(cat | sst_csf)
    sst_csf input
  else
    input="$@"
  fi
  readonly input

  output=
  for ((i = 0; i < ${#input}; ++i)); do
    c=${input:i:1}
    if [[ "$c" == '\' ]]; then
      output+=[\\$c]
    else
      output+=[$c]
    fi
  done
  readonly output

  printf '%s\n' "$output"

}; readonly -f sst_regex_escape
