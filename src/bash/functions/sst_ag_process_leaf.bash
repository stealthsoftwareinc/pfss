#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ag_process_leaf_helper() {

  local child
  local child_slug
  local children
  local children_nodist
  local leaf
  local leaves
  # Bash >=4.2: declare -g sst_ag_process_leaf_seen
  local target
  local target_slug

  sst_expect_argument_count $# 3

  target=$1
  readonly target
  sst_expect_source_path "$target"

  leaf=$2
  readonly leaf
  sst_expect_source_path "$leaf"

  sst_expect_basic_identifier "$3"

  target_slug=$(sst_underscore_slug $target)
  readonly target_slug

  children=${target_slug}_children
  readonly children

  children_nodist=${children}_nodist
  readonly children_nodist

  leaves=${target_slug}_leaves
  readonly leaves

  if [[ $leaf == *.phony.@(ag|ac|am) ]]; then
    child=
    sst_${leaf##*.}_include $leaf
  elif [[ $leaf == *.@(ag|ac|am) ]]; then
    child=${leaf%%.@(ag|ac|am)}
    if [[ "${sst_ag_process_leaf_seen= }" == *" $child "* ]]; then
      child=
    else
      child_slug=$(sst_underscore_slug $child)
      sst_${leaf##*.}_include $leaf
      sst_am_var_add_unique_file $children_nodist $child
      sst_am_var_add_unique_word $leaves "\$(${child_slug}_leaves)"
      # TODO: sst_am_var_add_unique_word $target/clean $child/clean
    fi
  elif [[ $leaf == *.@(im.in|in|im) ]]; then
    child=${leaf%%.@(im.in|in|im)}
    if [[ "${sst_ag_process_leaf_seen= }" == *" $child "* ]]; then
      child=
    else
      child_slug=$(sst_underscore_slug $child)
      sst_ac_config_file $leaf
      sst_am_var_add_unique_file $children_nodist $child
      sst_am_var_add_unique_word $leaves "\$(${child_slug}_leaves)"
      # TODO: sst_am_var_add_unique_word $target/clean $child/clean
    fi
  elif [[ $leaf == *.m4 ]]; then
    child=${leaf%%.m4}
    if [[ "${sst_ag_process_leaf_seen= }" == *" $child "* ]]; then
      child=
    else
      child_slug=$(sst_underscore_slug $child)
      sst_am_distribute_file $leaf
      sst_ac_append <<<"GATBPS_M4([$child])"
      sst_am_var_add_unique_file $children_nodist $child
      sst_am_var_add_unique_word $leaves "\$(${child_slug}_leaves)"
      # TODO: sst_am_var_add_unique_word $target/clean $child/clean
    fi
  else
    child=$leaf
    if [[ "${sst_ag_process_leaf_seen= }" == *" $child "* ]]; then
      child=
    else
      sst_am_distribute_file $leaf
      sst_am_var_add_unique_file $children $child
      sst_am_var_add_unique_file $leaves $leaf
    fi
  fi

  if [[ "$child" != "" ]]; then
    sst_ag_process_leaf_seen+="$child "
  fi

  printf '%s\n' $child

}; readonly -f sst_ag_process_leaf_helper

sst_ag_process_leaf() {
  sst_ag_process_leaf_helper "$@" >"$sst_root_tmpdir"/$FUNCNAME.child
  eval $3='$(cat "$sst_root_tmpdir"/$FUNCNAME.child)'
}; readonly -f sst_ag_process_leaf
