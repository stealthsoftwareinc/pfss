#! /bin/sh -
#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#
# This script uses the SST Bash library to bootstrap from /bin/sh into
# bash. See the Bash library > Bootstrapping section of the SST manual
# for more information.
#

#-----------------------------------------------------------------------
# Include the SST Bash library
#-----------------------------------------------------------------------
#
# Include the first sst.bash file found by searching up the directory
# tree starting from the location of this script file. If no such file
# is found, fall back to including plain sst.bash with PATH resolution.
# Use SST_BASH_PRELUDE_PATH to cache the location of the sst.bash file
# to prevent a redundant search from occurring during the bootstrap.
#

case ${SST_BASH_PRELUDE_PATH-} in '')
  case $0 in /*)
    x=$0
  ;; *)
    x=./$0
  esac
  r='\(.*/\)'
  x=`expr "x${x?}" : "x${r?}"`. || exit $?
  SST_BASH_PRELUDE_PATH=`
    cd "${x?}" || exit $?
    while :; do
      if test -f sst.bash; then
        case ${PWD?} in *[!/]*)
          printf '%s\n' "${PWD?}"/sst.bash || exit $?
        ;; *)
          printf '%s\n' "${PWD?}"sst.bash || exit $?
        esac
        exit
      fi
      case ${PWD?} in *[!/]*)
        cd ..
      ;; *)
        printf '%s\n' sst.bash || exit $?
        exit
      esac
    done
  ` || exit $?
  export SST_BASH_PRELUDE_PATH
esac
set -e || exit $?
. "${SST_BASH_PRELUDE_PATH?}"
unset SST_BASH_PRELUDE_PATH

#-----------------------------------------------------------------------

function main {

  local branch
  local dst_branch
  local dst_tag
  local full_ref
  local -r github_ssh_regex='^(ssh://)?([^@]+@)?(([^:]+\.)?github\.[^:]+):(.*)\.git$'
  local -r gitlab_ssh_regex='^(ssh://)?([^@]+@)?(([^:]+\.)?gitlab\.[^:]+):(.*)\.git$'
  local ref_already_exists
  local short_ref
  local tag_transform
  local -a tags
  local x
  local y

  sst_expect_argument_count $# 1

  args=$1
  readonly args

  #---------------------------------------------------------------------

  sst_jq_get_string "$args" .gitbundle gitbundle
  if [[ "$gitbundle" == "" ]]; then
    gitbundle=preview.gitbundle
  fi
  readonly gitbundle

  sst_jq_get_strings "$args" .pull_repo pull_repo
  readonly pull_repo

  sst_jq_get_string "$args" .pull_commit pull_commit
  readonly pull_commit

  sst_jq_get_string "$args" .push_from_server push_from_server
  readonly push_from_server

  sst_jq_get_string "$args" .push_from_project push_from_project
  readonly push_from_project

  sst_jq_get_strings "$args" .push_from_branch push_from_branch
  readonly push_from_branch

  sst_jq_get_strings "$args" .push_from_tag push_from_tag
  readonly push_from_tag

  sst_jq_get_strings "$args" .push_repo push_repo
  readonly push_repo

  sst_jq_get_string "$args" .update_script update_script
  readonly update_script

  sst_jq_get_string "$args" .post_update_script post_update_script
  readonly post_update_script

  sst_jq_get_string "$args" .commit_script commit_script
  readonly commit_script

  sst_jq_get_string "$args" .post_commit_script post_commit_script
  readonly post_commit_script

  sst_jq_get_string "$args" .dist_archive dist_archive

  sst_jq_get_string "$args" tag_transform
  readonly tag_transform

  #---------------------------------------------------------------------

  #
  # Get the distribution archive.
  #

  case $dist_archive in
    '')
      dist_archive=$sst_rundir/$(sst_find_dist_archive | sst_csf)
      sst_csf dist_archive
    ;;
    git-archive)
      dist_archive=$tmpdir/dist.tar
      git archive --prefix=x/ HEAD >"$dist_archive"
    ;;
  esac
  readonly dist_archive

  #---------------------------------------------------------------------

  while :; do
    for url in "${pull_repo[@]}"; do
      if [[ "$url" ]]; then
        rm -f -r "$tmpdir"/pull_repo
        eval "url=\"$url\""
        if git clone -- "$url" "$tmpdir"/pull_repo; then
          if [[ ! "$pull_commit" ]]; then
            break 2
          else
            sst_pushd "$tmpdir"/pull_repo
            if git checkout "$pull_commit^{commit}"; then
              sst_popd
              break 2
            fi
            sst_popd
          fi
        fi
      fi
    done
    sst_barf 'there were no successful clone attempts'
  done

  cd "$tmpdir"/pull_repo

  #---------------------------------------------------------------------
  # Get the branch name of the cloned pull repository
  #---------------------------------------------------------------------

  n=$(git rev-list --branches | awk 'END { print NR }')
  if ((n == 0)); then
    sst_barf '%s: expected exactly one branch' "$pull_repo"
  fi

  branch=$(git show-ref --heads | awk '{ print $2 }')
  readonly branch

  if [[ "$branch" == *$'\n'* ]]; then
    sst_barf '%s: expected exactly one branch' "$pull_repo"
  fi

  #---------------------------------------------------------------------
  #
  # Extract the distribution archive to an empty temporary directory.
  # We'll be giving the extracted directory to the update script.
  #

  x=$tmpdir/dist_archive
  mkdir "$x"
  (
    cd "$x"
    sst_extract_archive "$dist_archive"
  )
  for x in "$x"/*; do
    dist_dir=$x
  done
  readonly dist_dir

  #---------------------------------------------------------------------
  # Update script
  #---------------------------------------------------------------------

  if [[ "$update_script" ]]; then
    "$sst_rundir/$update_script" \
      "$dist_dir" \
      "$sst_rundir" \
    ;
  else

    #
    # When no update script is specified, perform the default update
    # script behavior, which simply replaces the entire content of the
    # repository with the entire content of the extracted distribution
    # archive directory.
    #

    files=$(git ls-files)
    if [[ "$files" != "" ]]; then
      git rm '*'
    fi
    cp -R "$dist_dir"/* .
    git add .

  fi

  #---------------------------------------------------------------------
  # Post-update script
  #---------------------------------------------------------------------

  if [[ "$post_update_script" ]]; then
    "$sst_rundir/$post_update_script" "$dist_dir" "$sst_rundir"
  fi

  #---------------------------------------------------------------------

  dst_tag=${CI_COMMIT_TAG-}
  if [[ "$dst_tag" && "$tag_transform" ]]; then
    dst_tag=$(sed " $tag_transform" <<<"$dst_tag" | sst_csf)
    sst_csf dst_tag
    if [[ ! "$dst_tag" ]]; then
      x=$(sst_quote " $tag_transform")
      y=$(sst_quote "$dst_tag")
      sst_barf \
        "The tag transform (sed $x <<<$y) produced an empty result." \
      ;
    fi
  fi
  readonly dst_tag

  #---------------------------------------------------------------------
  # Commit
  #---------------------------------------------------------------------

  if [[ "$commit_script" ]]; then
    "$sst_rundir/$commit_script" \
      "$dist_dir" \
      "$sst_rundir" \
    ;
  else
    git commit \
      --allow-empty \
      -m "Update to ${dst_tag:-$CI_COMMIT_SHA}" \
    ;
  fi

  #---------------------------------------------------------------------

  if [[ "$dst_tag" ]]; then
    dst_branch=
    git tag ${GPG_SECRET_KEY:+-s} -m "$dst_tag" -- "$dst_tag"
  else
    dst_branch=preview-$(
      TZ=UTC git log \
        --date='format-local:%Y-%m-%d-%H%M%S' \
        --pretty='%ad' \
        -1 \
      ;
    )-$CI_COMMIT_SHA
    git branch -- "$dst_branch"
  fi
  readonly dst_branch

  #---------------------------------------------------------------------
  # Post-commit script
  #---------------------------------------------------------------------

  if [[ "$post_commit_script" ]]; then
    "$sst_rundir/$post_commit_script" "$dist_dir" "$sst_rundir"
  fi

  #---------------------------------------------------------------------
  # List the tags
  #---------------------------------------------------------------------

  tags=$(git tag)
  if [[ "$tags" ]]; then
    tags='tags=(); '$(git show-ref --tags | sed '
      s/'\''/&\\&&/g
      s/^.* /tags+=('\''/
      s/$/'\'')/
    ')
    eval "$tags"
  else
    tags=()
  fi
  readonly tags

  #---------------------------------------------------------------------
  # Create the gitbundle
  #---------------------------------------------------------------------

  git bundle create "$sst_rundir/$gitbundle" HEAD "$branch" "${tags[@]}"

  #---------------------------------------------------------------------
  # Decide whether we should push
  #---------------------------------------------------------------------

  should_push_a=0
  if [[ \
       "$push_from_server" \
    && "$push_from_project" \
    && "$push_from_server" == "${CI_SERVER_HOST-}" \
    && (   "$push_from_project" == "${CI_PROJECT_ID-}" \
        || "$push_from_project" == "${CI_PROJECT_PATH-}") \
  ]]; then
    should_push_a=1
  fi
  readonly should_push_a

  should_push_b=0
  if ((should_push_a)); then
    if [[ "${CI_COMMIT_BRANCH-}" ]]; then
      for x in ${push_from_branch[@]+"${push_from_branch[@]}"}; do
        if [[ "$x" == /*/ ]]; then
          x=${x#?}
          x=${x%?}
          if [[ "$CI_COMMIT_BRANCH" =~ $x ]]; then
            should_push_b=1
          fi
        elif [[ "$CI_COMMIT_BRANCH" == "$x" ]]; then
          should_push_b=1
        fi
      done
    elif [[ "${CI_COMMIT_TAG-}" ]]; then
      for x in ${push_from_tag[@]+"${push_from_tag[@]}"}; do
        if [[ "$x" == /*/ ]]; then
          x=${x#?}
          x=${x%?}
          if [[ "$CI_COMMIT_TAG" =~ $x ]]; then
            should_push_b=1
          fi
        elif [[ "$CI_COMMIT_TAG" == "$x" ]]; then
          should_push_b=1
        fi
      done
    fi
  fi
  readonly should_push_b

  ((should_push = should_push_a && should_push_b, 1))
  readonly should_push

  #---------------------------------------------------------------------
  # Push
  #---------------------------------------------------------------------

  push_types=
  view_urls=()

  if ((should_push)); then
    for url in ${push_repo[@]+"${push_repo[@]}"}; do

      if [[ ! "$url" ]]; then
        continue
      fi

      if [[ "$dst_branch" ]]; then
        short_ref=$dst_branch
        full_ref=refs/heads/$short_ref
      else
        short_ref=$dst_tag
        full_ref=refs/tags/$short_ref
      fi

      ref_already_exists=$(git ls-remote -- "$url" "$full_ref")
      if [[ "$ref_already_exists" ]]; then
        p='*'
      else
        p=' '
        git push -- "$url" "$full_ref:$full_ref"
      fi

      push_types+=$p

      if ((0)); then
        :
      elif [[ "$url" =~ $github_ssh_regex ]]; then
        view_urls+=("$p https://${BASH_REMATCH[3]}/${BASH_REMATCH[5]}/tree/$short_ref")
      elif [[ "$url" =~ $gitlab_ssh_regex ]]; then
        view_urls+=("$p https://${BASH_REMATCH[3]}/${BASH_REMATCH[5]}/-/tree/$short_ref")
      fi

    done
  fi

  readonly push_types
  readonly view_urls

  printf '\n'
  if ((!should_push)); then
    m='The preview commit was not pushed anywhere because'
    m+=' .push_from_* was not satisfied.'
    printf '%s\n' "$m"
  elif [[ ! "$push_types" ]]; then
    m='The preview commit was not pushed anywhere because'
    m+=' .push_repo was empty.'
    printf '%s\n' "$m"
  elif ((${#view_urls[@]} == 0)); then
    m='The preview commit was pushed to all .push_repo entries'
    m+=', but there are no known URLs at which to view it.'
    printf '%s\n' "$m"
  else
    m='The preview commit was pushed to all .push_repo entries'
    m+='. You can view it at the following URLs'
    if [[ "$push_types" == *'*'* ]]; then
      m+=' ("*" means "pushed by a previous job")'
    fi
    m+=':'
    printf '%s\n\n' "$m"
    printf '   %s\n' "${view_urls[@]}"
  fi
  printf '\n'

}; readonly -f main

main "$@"
