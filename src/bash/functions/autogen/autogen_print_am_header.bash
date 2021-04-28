#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# autogen_print_am_header
#

autogen_print_am_header() {

  if (($# != 0)); then
    sst_barf 'invalid argument count: %d' $#
  fi

  cat <<"EOF"
##
## For the copyright information for this file, please search up the
## directory tree for the first COPYING file.
##

##
## This file was generated by autogen.
##

EOF

}; readonly -f autogen_print_am_header