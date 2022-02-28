#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_test() {

  declare     checks
  declare     code
  declare     expected_output
  declare     expected_output_op
  declare     expected_status
  declare     output
  declare     pass
  declare     prelude
  declare     regex
  declare     script
  declare     script_file
  declare     shows
  declare     squish
  declare     status
  declare     x
  declare     y

  squish='
    /^ *$/d
    s/^ *//
    s/ *$//
  '
  readonly squish

  expected_status=0
  expected_output_op=:
  expected_output=
  regex='^(0|[1-9][0-9]*)((:|=)(.*))?$'
  if [[ $# != 0 && "$1" =~ $regex ]]; then
    expected_status=${BASH_REMATCH[1]}
    expected_output_op=${BASH_REMATCH[3]:-:}
    expected_output=${BASH_REMATCH[4]-}
    shift
  fi

  if [[ $# == 0 ]]; then
    sst_barf "<code> must be given."
  fi
  code=$(
    sed "$squish" <<<"$1" | tr '\n' ' ' | sed "$squish" | sst_csf
  )
  sst_csf code
  code=${code%';'}
  readonly code
  shift

  checks=
  shows=
  if [[ $# != 0 ]]; then
    checks=$(
      sed "$squish" <<<"$1" | tr '\n' ' ' | sed "$squish" | sst_csf
    )
    sst_csf checks
    checks=${checks%';'}
    readonly checks
    shift
    for x; do
      shows+=' '$(sst_quote "$x")
    done
  fi
  readonly checks
  readonly shows

  prelude=$(sst_quote "$sst_sstdir/sst.bash")
  readonly prelude

  script='
    set -e || exit $?
    . '"$prelude"'
    '"$code"'
    {
      sst_test_status=$?
      '"$checks"'
    } || {
      sst_test_show=('"$shows"')
      sst_test_fail "$@"
    }
    (exit $sst_test_status) && :
  '
  readonly script

  x="Test: ({ $code; }; $checks) exits with status $expected_status"
  if [[ "$expected_output_op" != : || "$expected_output" != "" ]]; then
    y=$(sst_quote "$expected_output")
    case $expected_output_op in
      :) y="that contains $y" ;;
      =) y="that is exactly $y" ;;
      *) sst_barf "missing case" ;;
    esac
    x+=" and produces output $y"
  fi
  x+=" ..."
  printf '%s' "$x"

  script_file=$sst_root_tmpdir/$FUNCNAME.$BASHPID.script
  readonly script_file

  cat <<<"$script" >"$script_file"
  status=0
  output=$(sh "$script_file" 2>&1 | sst_csf) || status=$?
  sst_csf output
  readonly status
  readonly output

  pass=
  if [[ $status == $expected_status ]]; then
    case $expected_output_op in
      :)
        if [[ "$output" == *"$expected_output"* ]]; then
          pass=1
        fi
      ;;
      =)
        if [[ "$output" == "$expected_output" ]]; then
          pass=1
        fi
      ;;
      *)
        sst_barf "missing case"
      ;;
    esac
  fi
  readonly pass

  if ((pass)); then
    printf '%s\n' " pass"
  else
    printf '%s\n' " fail" "  status: $status" "$output"
    exit 1
  fi

}; readonly -f sst_test
