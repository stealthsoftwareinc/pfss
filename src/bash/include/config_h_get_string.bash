#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

config_h_get_string() {

  local config
  local macro
  local main

  case $# in
    2)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  config=$1
  readonly config
  expect_safe_path "$config"

  macro=$2
  readonly macro
  sst_expect_basic_identifier "$macro"

  main=config_h_get_string-$macro-$$.c
  readonly main

  cat >$main <<EOF
#include "$config"
#include <stdio.h>
#include <stdlib.h>
int (main)(
  void
) {
  static char const * const x = $macro;
  if (printf("%s\\n", x) >= 0) {
    if (fflush(stdout) == 0) {
      return EXIT_SUCCESS;
    }
  }
  return EXIT_FAILURE;
}
EOF

  cc -o $main.exe $main

  ./$main.exe

  rm $main.exe $main

}; readonly -f config_h_get_string
