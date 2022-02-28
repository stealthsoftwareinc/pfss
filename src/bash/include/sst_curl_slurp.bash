#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_curl_slurp() {

  declare a
  declare b
  declare get_next
  declare headers
  declare url

  a=$'[\t ]*'
  readonly a

  b=$'[^\t >][^\t >]*'
  readonly b

  get_next=
  get_next+="s/^$a[Ll][Ii][Nn][Kk]$a:.*<$a\\($b\\)$a>$a;"
  get_next+="$a[Rr][Ee][Ll]$a=$a\"$a[Nn][Ee][Xx][Tt]$a\".*/\\1/p"
  readonly get_next

  headers=$sst_root_tmpdir/$FUNCNAME.$BASHPID.headers
  readonly headers

  sst_expect_argument_count $# 1-

  url=$1
  shift

  while [[ "$url" ]]; do
    curl -D "$headers" "$@" -- "$url"
    url=$(tr -d '\r' <"$headers" | sed -n "$get_next")
  done

}; readonly -f sst_curl_slurp
