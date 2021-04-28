#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ajh_build_tree_program_wrapper() {

  local ag_json
  local wrappee
  local wrapper

  for ag_json; do

    sst_expect_ag_json wrapper "$ag_json"

    sst_jq_get_string $ag_json .wrappee wrappee
    sst_expect_source_path "$wrappee"

    sst_mkdir_p_only $wrapper.im

    cat >$wrapper.im <<EOF
#! /bin/sh -
#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

wrappee=\`cat <<"/../"
{@}abs_builddir{@}/$wrappee
/../
\` || exit \$?

case \$# in
  0)
    USE_BUILD_TREE=1 exec "\$wrappee"
  ;;
  *)
    USE_BUILD_TREE=1 exec "\$wrappee" "\$@"
  ;;
esac
EOF

    chmod +x $wrapper.im

    autogen_ac_append <<EOF

]dnl $wrapper
GATBPS_CONFIG_LATER([$wrapper])
[

EOF

    sst_am_append <<EOF

## $wrapper
EXTRA_DIST += $wrapper.im
noinst_SCRIPTS += $wrapper

EOF

  done

}; readonly -f sst_ajh_build_tree_program_wrapper
