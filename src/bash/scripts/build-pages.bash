#! /bin/sh -
#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#
# This script uses the SST Bash library to bootstrap from /bin/sh into
# bash. See the Bash library > Bootstrapping section of the SST manual
# for more information.
#

case $0 in /*) x=$0 ;; *) x=./$0 ;; esac
r='\(.*/\)'
x=`expr "$x" : "$r"` || exit $?
set -e || exit $?
. "$x"../prelude.bash

#-----------------------------------------------------------------------

dist_archive=$(sst_find_dist_archive)
readonly dist_archive

mkdir "$sst_tmpdir"/x
sst_pushd "$sst_tmpdir"/x
sst_extract_archive "$sst_rundir"/"$dist_archive"
sst_popd

mkdir "$sst_tmpdir"/y
sst_pushd "$sst_tmpdir"/y
sst_extract_archive "$sst_tmpdir"/x/*/doc/*-pages.tar.*
sst_popd

# TODO: Use sst_mkdir_p_new_only after it's implemented.
mkdir public
rmdir public
mv -f "$sst_tmpdir"/y/* public
