#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_get_max_procs() {

  # Bash >=4.2: declare -g sst_max_procs

  declare    n

  if (($# > 0)); then
    sst_expect_argument_count $# 0
  fi

  if [[ "${sst_max_procs+x}" ]]; then
    return
  fi

  sst_max_procs=1

  if [[ -f /proc/cpuinfo ]]; then
    n=$(
      awk '
        BEGIN {
          n = 0;
        }
        /^processor[\t ]*:/ {
          ++n;
        }
        END {
          print n;
        }
      ' /proc/cpuinfo
    )
    if ((n > 0)); then
      sst_max_procs=$n
    fi
  fi

  readonly sst_max_procs

}; readonly -f sst_get_max_procs
