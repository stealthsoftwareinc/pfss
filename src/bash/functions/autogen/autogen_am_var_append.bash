#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# autogen_am_var_append <name> [<text>...]
#

autogen_am_var_append() {

  local name
  local text

  if (( $# == 0 )); then
    sst_barf 'invalid argument count: %d' $#
  fi

  name=$1
  readonly name
  shift
  sst_expect_basic_identifier "$name"

  for text; do
    sst_am_append <<EOF
$name += $text
EOF
  done

}; readonly -f autogen_am_var_append
