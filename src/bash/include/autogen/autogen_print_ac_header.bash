#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

#
# autogen_print_ac_header
#

autogen_print_ac_header() {

  if (($# != 0)); then
    sst_barf 'invalid argument count: %d' $#
  fi

  cat <<"EOF"
dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

dnl
dnl This file was generated by autogen.
dnl

EOF

}; readonly -f autogen_print_ac_header