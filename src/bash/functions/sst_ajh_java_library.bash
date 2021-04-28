#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# .am_df_INPUT
#
# An optional file to which to write the list of .java files. The file
# will be suitable to be processed by the .am_df_INPUT.df makefile rule.
#
# jardeps
#
# An optional array of .jar file names that dst depends on. These files
# will be expected to exist in ${javadir} or /usr/local/share/java or
# /usr/share/java at build time and after installation.
#
# If you have makefile targets for some of these files, you can list the
# targets instead of just their file names. For example, you could list
# build-aux/downloads/foobar.jar instead of foobar.jar. A target x is
# detected by the existence of a corresponding x.ag.*, x.ac, or x.am
# file. When one or more of these targets are detected, several
# installation targets are generated:
#
#       install-java-$(slug)-jardeps-targets
#
#             Makes all targeted jardeps for dst.
#
#       install-java-$(slug)-jardeps
#
#             Installs all targeted jardeps for dst.
#
#       install-java-jardeps-targets
#
#             Makes all targeted jardeps for every dst,
#             even those for which noinst is true.
#
#       install-java-jardeps
#
#             Installs all targeted jardeps for every dst,
#             even those for which noinst is true.
#
# Do not use jardeps to list .jar dependencies that are built by this
# project. Use built_jardeps instead.
#
# built_jardeps
#
# An optional array of .jar makefile targets that dst depends on that
# are built by this project.
#

sst_ajh_java_library() {

  local ag
  local built_jardeps
  local child
  local dst
  local dst_slug
  local ext
  local jardeps
  local java_slug
  local java_target_slug
  local javac_cp
  local javadoc_slug
  local leaf
  local maven_sourcepath
  local noinst
  local package
  local package_dir
  local slug
  local sourcepath
  local x
  local xs
  local y
  local y3

  for ag; do

    expect_safe_path "$ag"
    case $ag in
      *.jar.ag.json)
      ;;
      *)
        sst_barf 'expected *.jar.ag.json: %s' $ag
      ;;
    esac

    jq_expect_object . $ag

    jq_expect_string .dst $ag
    dst=$(jq -r .dst $ag)
    expect_safe_path "$dst"
    case $dst in
      *.jar)
      ;;
      *)
        sst_barf '%s: .dst: expected *.jar: %s' $ag $dst
      ;;
    esac

    dst_slug=$(sst_underscore_slug $dst)

    jq_expect_string_or_null .slug $ag
    slug=$(jq -r '.slug | select(.)' $ag)
    readonly slug

    jq_expect_strings_or_null .jardeps $ag
    jardeps=$(jq -r '.jardeps | select(.) | .[]' $ag)
    readonly jardeps

    jq_expect_strings_or_null .built_jardeps $ag
    built_jardeps=$(jq -r '.built_jardeps | select(.) | .[]' $ag)
    readonly built_jardeps

    javac_cp=
    x=$(basename $dst)
    printf '{@}javadir{@}/%s' "$x" >$dst.classpath.im.in
    printf '{@}abs_builddir{@}/%s' "$dst" >$dst.build_tree_classpath.im.in
    for x in $jardeps; do
      x=$(basename $x)
      case $javac_cp in
        ?*)
          javac_cp=$javac_cp\$\(CLASSPATH_SEPARATOR\)
        ;;
      esac
      javac_cp=$javac_cp\${javadir}/$x
      javac_cp=$javac_cp\$\(CLASSPATH_SEPARATOR\)
      javac_cp=$javac_cp/usr/local/share/java/$x
      javac_cp=$javac_cp\$\(CLASSPATH_SEPARATOR\)
      javac_cp=$javac_cp/usr/share/java/$x
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.build_tree_classpath.im.in
      printf '{@}javadir{@}/%s' "$x" >>$dst.build_tree_classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.build_tree_classpath.im.in
      printf '/usr/local/share/java/%s' "$x" >>$dst.build_tree_classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.build_tree_classpath.im.in
      printf '/usr/share/java/%s' "$x" >>$dst.build_tree_classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.classpath.im.in
      printf '{@}javadir{@}/%s' "$x" >>$dst.classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.classpath.im.in
      printf '/usr/local/share/java/%s' "$x" >>$dst.classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.classpath.im.in
      printf '/usr/share/java/%s' "$x" >>$dst.classpath.im.in
    done
    for x in $built_jardeps; do
      case $javac_cp in
        ?*)
          javac_cp=$javac_cp\$\(CLASSPATH_SEPARATOR\)
        ;;
      esac
      javac_cp=$javac_cp$x
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.build_tree_classpath.im.in
      printf '{@}abs_builddir{@}/%s' "$x" >>$dst.build_tree_classpath.im.in
      x=$(basename $x)
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.classpath.im.in
      printf '{@}javadir{@}/%s' "$x" >>$dst.classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.classpath.im.in
      printf '/usr/local/share/java/%s' "$x" >>$dst.classpath.im.in
      printf '@CLASSPATH_SEPARATOR_RAW_LATER@' >>$dst.classpath.im.in
      printf '/usr/share/java/%s' "$x" >>$dst.classpath.im.in
    done
    readonly javac_cp
    printf '\n' >>$dst.classpath.im.in

    case $slug in
      "")
        java_target_slug=java-main
        java_slug=java
        javadoc_slug=javadoc
      ;;
      *)
        java_target_slug=java-$slug
        java_slug=java_$slug
        javadoc_slug=javadoc_$slug
      ;;
    esac
    readonly java_target_slug
    readonly java_slug
    readonly javadoc_slug

    jq_expect_string '.sourcepath' $ag
    sourcepath=$(jq -r '.sourcepath' $ag)
    expect_safe_path "$sourcepath"

    sst_jq_get_string "$ag" .maven_sourcepath maven_sourcepath
    readonly maven_sourcepath
    if [[ "$maven_sourcepath" != "" ]]; then
      sst_expect_source_path "$maven_sourcepath"
    fi

    package=$(jq -r .package $ag)
    readonly package

    package_dir=$(echo $package | sed 's|\.|/|g')
    readonly package_dir

    noinst=$(jq -r 'select(.noinst == true) | "x"' $ag)
    readonly noinst

    autogen_ac_append <<EOF

]GATBPS_CONFIG_FILE([$dst.classpath.im])[
]GATBPS_CONFIG_LATER([$dst.classpath])[

]GATBPS_CONFIG_FILE([$dst.build_tree_classpath.im])[
]GATBPS_CONFIG_LATER([$dst.build_tree_classpath])[

EOF

    case $slug in
      ?*)
        autogen_ac_append <<EOF

]GATBPS_JAVA([$slug])[

EOF
      ;;
    esac

    sst_am_append <<EOF
${java_slug}_CLASSPATH = $javac_cp
${java_slug}_dep =
$dst: $built_jardeps
${java_target_slug}: $dst.classpath
${java_target_slug}: $dst.build_tree_classpath
${java_slug}_dst = $dst
${java_slug}_nested =
${java_slug}_noinst = $noinst
${java_slug}_package = $package
${java_slug}_sourcepath = $sourcepath
${java_slug}_src =
${javadoc_slug}_src =
EXTRA_DIST += \$(${javadoc_slug}_src)
jar_classpath_files += $dst.classpath
EOF

    sst_am_append <<EOF

install-$java_target_slug-jardeps-targets: FORCE
install-$java_target_slug-jardeps: FORCE
install-java-jardeps-targets: FORCE
install-java-jardeps: FORCE

.PHONY: install-$java_target_slug-jardeps-targets
.PHONY: install-$java_target_slug-jardeps
.PHONY: install-java-jardeps-targets
.PHONY: install-java-jardeps

install-java-jardeps-targets: install-$java_target_slug-jardeps-targets

install-java-jardeps: install-$java_target_slug-jardeps
EOF

    for x in $jardeps; do
      for y in $x.ag.* $x.ac $x.am; do
        if test -f $y; then
          sst_am_append <<EOF

install-$java_target_slug-jardeps-targets: $x

install-$java_target_slug-jardeps-$x: FORCE
install-$java_target_slug-jardeps-$x: $x
	@\$(NORMAL_INSTALL)
	\$(MKDIR_P) '\$(DESTDIR)\$(javadir)'
	\$(INSTALL_DATA) $x '\$(DESTDIR)\$(javadir)'

.PHONY: install-$java_target_slug-jardeps-$x

install-$java_target_slug-jardeps: install-$java_target_slug-jardeps-$x
EOF
          break
        fi
      done
    done

    xs=$(jq -r 'select(.dep) | .dep[]' $ag)
    for x in $xs; do
      sst_am_append <<EOF
${java_slug}_dep += $x
EOF
    done

    for x in $sourcepath/$package_dir/**/; do
      sst_ihs <<<"
        ${java_slug}_nested += $x*\\\$\$*.class
      " | sst_am_append
    done

    ext='@(.ag|.ac|.am|.im.in|.in|.im|.m4|)'
    for leaf in $sourcepath/$package_dir/**/*.java$ext; do

      sst_expect_source_path "$leaf"

      sst_ag_process_leaf $dst/src $leaf child

      if [[ "$child" == "" ]]; then
        continue
      fi

      if [[ "$maven_sourcepath" != "" ]]; then
        x=${child/#$sourcepath/$maven_sourcepath}
        sst_ac_append <<<"GATBPS_CP([$x], [$child])"
        sst_am_append <<<"maven-prep: $x"
      fi

      y3=${child/%.java/.class}

      autogen_ac_append <<EOF

]GATBPS_JAVA_CLASS(
  [$y3],
  [mostlyclean])[

EOF

      sst_ihs <<<"
        ${java_slug}_src += $y3
      " | sst_am_append

      sst_ihs <<<"
        ${javadoc_slug}_src += $child
      " | sst_am_append

    done

  done

}; readonly -f sst_ajh_java_library
