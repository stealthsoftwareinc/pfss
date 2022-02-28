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

  local commit
  local -r github_ssh_regex='^(ssh://)?([^@]+@)?(([^:]+\.)?github\.[^:]+):(.*)\.git$'
  local -r gitlab_ssh_regex='^(ssh://)?([^@]+@)?(([^:]+\.)?gitlab\.[^:]+):(.*)\.git$'

  sst_expect_argument_count $# 1

  if [[ "$1" == '{'* ]]; then
    args=$tmpdir/args.json
    cat <<<"$1" >"$args"
  else
    args=$1
  fi
  readonly args

  #
  # Parse .gitbundle from the arguments.
  #

  sst_jq_get_string_or_null .gitbundle "$args" gitbundle
  if [[ "$gitbundle" == "" ]]; then
    gitbundle=preview.gitbundle
  fi
  readonly gitbundle

  #
  # Parse .push_repo from the arguments.
  #

  unset push_repo
  unset error
  x=$(jq -r '
    .push_repo
    | if type == "array" then
        if length == 0 then
          "error+=(\": Expected a nonempty string"
                   + " or a nonempty array of nonempty strings.\")"
        else
          .[]
          | if type != "string" or . == "" then
              "error+=(\"[]: Expected a nonempty string.\")"
            else
              "push_repo+=(" + (. | @sh) + ")"
            end
        end
      elif type != "string" or . == "" then
        "error+=(\": Expected a nonempty string"
                 + " or a nonempty array of nonempty strings.\")"
      else
        "push_repo+=(" + (. | @sh) + ")"
      end
  ' "$args")
  eval "$x"
  readonly push_repo
  if [[ "${error-}" != "" ]]; then
    sst_barf '%s: .push_repo%s' "$args" "$error"
  fi

  #
  #

  git clone -- "$gitbundle" "$tmpdir/repo"
  cd "$tmpdir/repo"

  #
  # Get the branch name of the gitbundle.
  #

  n=$(git rev-list --branches | awk 'END { print NR }')
  if ((n == 0)); then
    sst_barf '%s: expected exactly one branch' "$gitbundle"
  fi
  branch=$(git show-ref --heads | awk '{ print $2 }')
  readonly branch
  if [[ "$branch" == *$'\n'* ]]; then
    sst_barf '%s: expected exactly one branch' "$gitbundle"
  fi

  commit=$(git rev-parse "$branch")
  readonly commit

  #
  # Push the branch and all tags to every push repo.
  #

  view_urls=()

  for url in ${push_repo[@]+"${push_repo[@]}"}; do

    git push --follow-tags -- "$url" "$branch:$branch"

    if [[ "$url" =~ $github_ssh_regex ]]; then
      view_urls+=("https://${BASH_REMATCH[3]}/${BASH_REMATCH[5]}/tree/$commit")
    elif [[ "$url" =~ $gitlab_ssh_regex ]]; then
      view_urls+=("https://${BASH_REMATCH[3]}/${BASH_REMATCH[5]}/-/tree/$commit")
    fi

  done

  readonly view_urls

  printf '\n'
  m='The commit and all reachable annotated tags were pushed to all '
  m+='.push_repo entries. You can view it at the following URLs'
  m+=':'
  printf '%s\n\n' "$m"
  printf '   %s\n' "${view_urls[@]}"
  printf '\n'

}; readonly -f main

main "$@"
