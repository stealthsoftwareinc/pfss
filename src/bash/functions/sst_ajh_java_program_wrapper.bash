#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ajh_java_program_wrapper() {

  local arg
  local class
  local dst
  local jar
  local jar_base
  local schema
  local src
  local x

  for arg; do

    sst_expect_ag_json dst "$arg"

    sst_jq_get_string "$arg" .dst x
    if [[ "$x" ]]; then
      dst=$x
    fi

    class=$(jq -r '.class | select(type == "string")' $arg)

    jar=$(jq -r '.jar | select(type == "string")' $arg)

    src=$(jq -r '.src | select(type == "string")' $arg)
    case $src in
      "")
        src=$sst_root_tmpdir/autogen_java_program.src
        cat >"$src" <<'EOF'
#! /bin/sh -

#
# We use -ea in default_javaflags because it's a good default approach:
# slower code with more diagnostics is often preferable to faster code
# with fewer diagnostics. When we really do want to go as fast as
# possible, we can use -da in JAVAFLAGS.
#
# We export CLASSPATH because this allows it to be overridden by using
# -cp in JAVAFLAGS. We could accomplish the same thing by using -cp in
# default_javaflags, but exporting CLASSPATH is easier.
#

case $JAVA in
  '')
    JAVA=java
  ;;
esac
readonly JAVA

x=
x=$x' -ea'
default_javaflags=$x
readonly default_javaflags

readonly JAVAFLAGS

case $CLASSPATH in
  ?*)
    CLASSPATH=$CLASSPATH@CLASSPATH_SEPARATOR_LATER@
  ;;
esac
case $USE_BUILD_TREE in
  '')
    CLASSPATH=$CLASSPATH`
      cat {@}javadir{@}/%jar_base%.classpath
    ` || exit $?
  ;;
  ?*)
    CLASSPATH=$CLASSPATH`
      cat {@}abs_builddir{@}/%jar%.build_tree_classpath
    ` || exit $?
  ;;
esac
readonly CLASSPATH
export CLASSPATH

case $# in
  0)
    eval exec "$JAVA" \
      "$default_javaflags" \
      "$JAVAFLAGS" \
      %class% \
    ;
  ;;
  *)
    eval exec "$JAVA" \
      "$default_javaflags" \
      "$JAVAFLAGS" \
      %class% \
      '"$@"' \
    ;
  ;;
esac
EOF
      ;;
    esac

    jar_base=$(basename $jar)

    sed "
      s|%class%|$class|g
      s|%jar%|$jar|g
      s|%jar_base%|$jar_base|g
    " <"$src" >$dst.im.in

    sst_ac_config_file $dst

    sst_ihs <<<"
      bin_SCRIPTS += $dst
    " | sst_am_append

  done

}; readonly -f sst_ajh_java_program_wrapper
