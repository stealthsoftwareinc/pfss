#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_extract_archive() {

  sst_expect_argument_count $# 1

  local -r x="$1"
  local y

  case $x in
    *.tar)
      tar xf "$x"
    ;;
    *.tar.gz)
      tar xzf "$x"
    ;;
    *.tar.xz)
      tar xJf "$x"
    ;;
    *.zip)
      unzip "$x"
    ;;
    *)
      y=$(sst_quote "$x" | sst_csf)
      sst_csf y
      sst_barf 'unknown archive file extension: %s' "$y"
    ;;
  esac

}; readonly -f sst_extract_archive
