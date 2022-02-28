#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_ag_define_ordering_macros() {

  declare    x
  declare    x_file
  declare    y
  declare    y_file
  declare -A y_to_xs
  declare    ys

  if (($# != 0)); then
    sst_expect_argument_count $# 0
  fi

  y_to_xs=()

  for x_file in m4/*.m4; do
    x=$x_file
    x=${x##*/}
    x=${x%.*}
    ys=$(
      sed -n '
        s/.*GATBPS_BEFORE(\[\$0\], \[\([^]]*\)\]).*/\1/p
      ' $x_file
    )
    for y in $ys; do
      y_to_xs[$y]+=" $x"
    done
  done

  for y in ${!y_to_xs[@]}; do
    y_file=m4/$y.m4
    if [[ ! -e $y_file ]]; then

      sst_info "Generating: $y_file"

      >$y_file

      sst_ihs <<<'
        dnl
        dnl Copyright (C) Stealth Software Technologies, Inc.
        dnl
        dnl For the complete copyright information, please see the
        dnl associated README file.
        dnl

        AC_DEFUN_ONCE(['$y'], [
        GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
        { :
      ' >>$y_file

      for x in ${y_to_xs[$y]}; do
        sst_ihs -2 <<<'
          GATBPS_REQUIRE(['$x'])
        ' >>$y_file
      done

      sst_ihs <<<'
        }])
      ' >>$y_file

    fi
  done

}; readonly -f sst_ag_define_ordering_macros
