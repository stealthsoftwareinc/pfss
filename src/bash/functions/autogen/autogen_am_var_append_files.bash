#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# autogen_am_var_append_files <name> [<path>...]
#

autogen_am_var_append_files() {

  local name
  local path

  name=$1
  shift

  for path; do
    if [[ ! -d "$path" ]]; then
      autogen_am_var_append "$name" "$path"
    fi
  done

}; readonly -f autogen_am_var_append_files
