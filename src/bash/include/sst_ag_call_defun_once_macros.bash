#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_ag_call_defun_once_macros() {

  declare    x
  declare    xs
  declare    y

  xs=$(sst_grep -l AC_DEFUN_ONCE m4/*.m4)
  readonly xs

  y=m4/CALL_DEFUN_ONCE_MACROS.m4
  readonly y

  sst_expect_not_exist "$y"

  >$y

  sst_ihs <<<'
    dnl
    dnl Copyright (C) Stealth Software Technologies, Inc.
    dnl
    dnl For the complete copyright information, please see the
    dnl associated README file.
    dnl

    AC_DEFUN_ONCE([CALL_DEFUN_ONCE_MACROS], [[
    ]GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))[
    { :
  ' >>$y

  sst_push_var IFS

  for x in $xs; do
    x=${x##*/}
    x=${x%.*}
    sst_ihs -2 <<<'
      ]GATBPS_REQUIRE(['"$x"'])[
    ' >>$y
  done

  sst_pop_var IFS

  sst_ihs <<<'
    }]])
  ' >>$y

  sst_ac_append <<<'CALL_DEFUN_ONCE_MACROS'

}; readonly -f sst_ag_call_defun_once_macros
