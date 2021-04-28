#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_array_from_zterm() {

  sst_expect_argument_count $# 1-

  sst_expect_basic_identifier "$1"

  if (($# == 1)); then
    eval '

      local '$1'_list
      local '$1'_script

      '$1'_script=$sst_root_tmpdir/$FUNCNAME.$$.script
      readonly '$1'_script

      if [[ ! -f "$'$1'_script" ]]; then
        sst_ihs <<<"
          set -e
          q=\\'\''
          for x; do
            x=\$q\${x//\$q/\$q\\\\\$q\$q}\$q
            printf \"%s\\\\n\" \"\$x\"
          done
        " >"$'$1'_script"
      fi

      '$1'_list=$(xargs -0 bash "$'$1'_script")
      readonly '$1'_list

      eval '$1'="($'$1'_list)"

    '
  else
    eval '

      local '$1'_fifo
      local '$1'_pid

      '$1'_fifo=$sst_root_tmpdir/$FUNCNAME.$$.fifo
      readonly '$1'_fifo

      rm -f "$'$1'_fifo"
      mkfifo "$'$1'_fifo"

      shift
      "$@" <&0 >"$'$1'_fifo" &
      '$1'_pid=$!
      readonly '$1'_pid

      sst_array_from_zterm '$1' <"$'$1'_fifo"

      wait $'$1'_pid

    '
  fi

}; readonly -f sst_array_from_zterm
