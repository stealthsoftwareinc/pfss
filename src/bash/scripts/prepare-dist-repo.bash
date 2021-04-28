#! /bin/sh -
#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#
# This script uses the SST Bash library to bootstrap from /bin/sh into
# bash. See the Bash library > Bootstrapping section of the SST manual
# for more information.
#

# Load the prelude.
case $0 in /*) x=$0 ;; *) x=./$0 ;; esac
r='\(.*/\)'
x=`expr "$x" : "$r"` || exit $?
set -e || exit $?
. "$x"../prelude.bash

sst_expect_argument_count $# 1

args=$1
readonly args

#-----------------------------------------------------------------------

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

#-----------------------------------------------------------------------

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

#-----------------------------------------------------------------------

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

#-----------------------------------------------------------------------
# Get the branch name of the cloned pull repository
#-----------------------------------------------------------------------

n=$(git rev-list --branches | awk 'END { print NR }')
if ((n == 0)); then
  sst_barf '%s: expected exactly one branch' "$pull_repo"
fi

branch=$(git show-ref --heads | awk '{ print $2 }')
readonly branch

if [[ "$branch" == *$'\n'* ]]; then
  sst_barf '%s: expected exactly one branch' "$pull_repo"
fi

#-----------------------------------------------------------------------
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

#-----------------------------------------------------------------------
# Update script
#-----------------------------------------------------------------------

if [[ "$update_script" ]]; then
  "$sst_rundir/$update_script" "$dist_dir" "$sst_rundir"
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

#-----------------------------------------------------------------------
# Post-update script
#-----------------------------------------------------------------------

if [[ "$post_update_script" ]]; then
  "$sst_rundir/$post_update_script" "$dist_dir" "$sst_rundir"
fi

#-----------------------------------------------------------------------
# Commit script
#-----------------------------------------------------------------------

if [[ "$commit_script" ]]; then
  "$sst_rundir/$commit_script" "$dist_dir" "$sst_rundir"
else

  if [[ "${CI_COMMIT_TAG-}" != "" ]]; then

    #
    # If we're building from a tag, reference the tag in our gitbundle
    # commit and duplicate the tag in our gitbundle. Also sign the tag
    # if we have a GPG key.
    #

    git commit --allow-empty -m "Update to $CI_COMMIT_TAG"
    s=${GPG_SECRET_KEY:+-s}
    git tag $s -m "$CI_COMMIT_TAG" -- "$CI_COMMIT_TAG"

  else

    #
    # If we're not building from a tag, just reference the commit hash
    # in our gitbundle commit.
    #

    git commit --allow-empty -m "Update to $CI_COMMIT_SHA"

  fi

fi

#-----------------------------------------------------------------------
# Post-commit script
#-----------------------------------------------------------------------

if [[ "$post_commit_script" ]]; then
  "$sst_rundir/$post_commit_script" "$dist_dir" "$sst_rundir"
fi

#-----------------------------------------------------------------------

#
# Create our gitbundle with the commit and all tags.
#

tags=$(git tag)
if [[ "$tags" == "" ]]; then
  tags=()
else
  tags=$(git show-ref --tags | sed '
    s/'\''/&\\&&/g
    s/^.* /tags+=('\''/
    s/$/'\'')/
    1s/^/unset tags;/
  ')
  eval "$tags"
fi
git bundle create "$sst_rundir/$gitbundle" HEAD "$branch" "${tags[@]}"

#
# Push the commit to every push repo as a new preview-* branch, but do
# not push the tags. The tags only go into the gitbundle. They will be
# pushed by a future publish-dist-repo job when the maintainer chooses
# to publish.
#

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
while ((should_push_a)); do
  if [[ "${CI_COMMIT_BRANCH-}" ]]; then
    for x in ${push_from_branch[@]+"${push_from_branch[@]}"}; do
      if [[ "$x" == /*/ ]]; then
        x=${x#?}
        x=${x%?}
        if [[ "$CI_COMMIT_BRANCH" =~ $x ]]; then
          should_push_b=1
          break 2
        fi
      elif [[ "$CI_COMMIT_BRANCH" == "$x" ]]; then
        should_push_b=1
        break 2
      fi
    done
  fi
  if [[ "${CI_COMMIT_TAG-}" ]]; then
    for x in ${push_from_tag[@]+"${push_from_tag[@]}"}; do
      if [[ "$x" == /*/ ]]; then
        x=${x#?}
        x=${x%?}
        if [[ "$CI_COMMIT_TAG" =~ $x ]]; then
          should_push_b=1
          break 2
        fi
      elif [[ "$CI_COMMIT_TAG" == "$x" ]]; then
        should_push_b=1
        break 2
      fi
    done
  fi
  break
done
readonly should_push_b

((should_push = should_push_a && should_push_b, 1))
readonly should_push

push_types=
view_urls=()

readonly github_ssh_regex='^(ssh://)?([^@]+@)?(([^:]+\.)?github\.[^:]+):(.*)\.git$'
readonly gitlab_ssh_regex='^(ssh://)?([^@]+@)?(([^:]+\.)?gitlab\.[^:]+):(.*)\.git$'

if ((should_push)); then
  for url in ${push_repo[@]+"${push_repo[@]}"}; do

    if [[ ! "$url" ]]; then
      continue
    fi

    if [[ "${CI_COMMIT_TAG-}" ]]; then
      b=$CI_COMMIT_TAG
    else
      b=$CI_COMMIT_SHA
    fi
    b=preview-$b
    dst_t=refs/tags/$b
    dst_b=refs/heads/$b

    p=

    dst_t_exists=$(git ls-remote -- "$url" "$dst_t")
    if [[ "$dst_t_exists" ]]; then
      p+='!'
    else
      p+=' '
    fi

    dst_b_exists=$(git ls-remote -- "$url" "$dst_b")
    if [[ "$dst_b_exists" ]]; then
      p+='*'
    else
      p+=' '
      git push -- "$url" HEAD:"$dst_b"
    fi

    push_types+=$p

    if [[ "$url" =~ $github_ssh_regex ]]; then
      view_urls+=("$p https://${BASH_REMATCH[3]}/${BASH_REMATCH[5]}/tree/$b")
    elif [[ "$url" =~ $gitlab_ssh_regex ]]; then
      view_urls+=("$p https://${BASH_REMATCH[3]}/${BASH_REMATCH[5]}/-/tree/$b")
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
  if [[ "$push_types" == *'!'* ]]; then
    m+=' ("!" means "branch/tag name collision")'
  fi
  if [[ "$push_types" == *'*'* ]]; then
    m+=' ("*" means "pushed by a previous job")'
  fi
  m+=':'
  printf '%s\n\n' "$m"
  printf '   %s\n' "${view_urls[@]}"
fi
printf '\n'
