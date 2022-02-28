#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

sst_ajh_build_tree_program_wrapper() {

  local ag_json
  local wrappee
  local wrapper

  for ag_json; do

    sst_expect_ag_json wrapper "$ag_json"

    sst_jq_get_string $ag_json wrappee
    if [[ ! "$wrappee" ]]; then
      wrappee=$wrapper.wrappee/${wrapper##*/}
    else
      sst_expect_source_path "$wrappee"
    fi

    sst_mkdir_p_only $wrapper.im

    sst_ihs <<<"
      #! /bin/sh -

      USE_BUILD_TREE=1
      readonly USE_BUILD_TREE
      export USE_BUILD_TREE

      wrappee='{@}abs_builddir{@}/$wrappee'
      readonly wrappee

      case \$# in 0)
        exec \"\$wrappee\"
      ;; *)
        exec \"\$wrappee\" \"\$@\"
      esac
    " >$wrapper.im

    chmod +x $wrapper.im

    sst_ac_config_file $wrapper.im

    sst_am_distribute $wrapper.im

    # TODO: Use some sst_am_var_add_* function that verifies that
    # $wrapper either exists as a file or doesn't exist, and does
    # deduplication?
    sst_am_var_add noinst_SCRIPTS $wrapper

  done

}; readonly -f sst_ajh_build_tree_program_wrapper
