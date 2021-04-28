#! /bin/sh -
#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#
# This script bootstraps into Bash. The shebang is only for portability.
# For more information, see the Bash library > Bootstrapping section of
# the SST manual.
#

# Load the prelude.
case $0 in /*) x=$0 ;; *) x=./$0 ;; esac
x=`dirname "$x"` || exit $?
case $x in /) x= ;; esac
set -e || exit $?
. "$x"/../../prelude.bash

sst_install_utility \
  cat \
  gawk \
  git \
  make \
  mv \
;

readonly dist_dir="$1"
readonly repo_dir="$2"

#-----------------------------------------------------------------------
# Temporary build tree
#-----------------------------------------------------------------------

build=$tmpdir/build
mkdir "$build"
rmdir "$build"
cp -R "$dist_dir" "$build"
(
  cd "$build"
  ./configure
)

#-----------------------------------------------------------------------
# README flattening
#-----------------------------------------------------------------------

gawk \
  -f "$repo_dir"/doc/readme/flatten.gawk \
  -v input_dir=. \
  -v script_file="$repo_dir"/doc/readme/flatten.gawk \
  README.adoc \
  >"$tmpdir"/README.adoc \
;
cat "$tmpdir"/README.adoc >README.adoc
git add README.adoc

#-----------------------------------------------------------------------
# License files
#-----------------------------------------------------------------------

year=$(date -u '+%Y')

cat <<EOF >COPYING
Copyright (C) 2015-$year Stealth Software Technologies, Inc.

This project is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This project is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this project.  If not, see <https://www.gnu.org/licenses/>.
EOF

git add COPYING

(cd "$build"; make build-aux/downloads/gpl-3.0.txt)
cat "$build"/build-aux/downloads/gpl-3.0.txt >COPYING.GPL3
git add COPYING.GPL3
