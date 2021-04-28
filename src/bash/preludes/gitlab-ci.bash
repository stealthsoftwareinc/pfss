#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# The preludes directory is DEPRECATED. There is only prelude.bash now.
#

#
# Bootstrap into the PATH-resolved bash. Don't unset SST_BASH_BOOTSTRAP
# because this would cause an infinite bootstrap loop with the prelude
# when we load it. Only the prelude does the unset.
#

case ${SST_BASH_BOOTSTRAP+x}y$# in
  y0) SST_BASH_BOOTSTRAP=x exec bash - "$0"      ;;
  y*) SST_BASH_BOOTSTRAP=x exec bash - "$0" "$@" ;;
esac

#
# Load the prelude.
#

x=$BASH_SOURCE
if [[ "$x" != */* ]]; then
  x=../prelude.bash
else
  x=${BASH_SOURCE%/*}/../prelude.bash
  if [[ "$x" != /* ]]; then
    x=./$x
  fi
fi
. "$x"
