#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ajh_asciidoctor_document() {

  local adoc
  local ag_json
  local clean_rule
  local distribute
  local html
  local imagesdir
  local prefix
  local s
  local seen
  local slug
  local tar_file
  local tar_file_slug
  local tarname
  local x
  local y

  for ag_json; do

    sst_expect_ag_json html "$ag_json"
    sst_expect_extension $ag_json .html.ag.json

    slug=$(sst_underscore_slug $html)

    adoc=${html%.html}.adoc
    sst_expect_any_file $adoc{,.ag.json,.ag,.ac,.am,.im.in,.in,.im}

    prefix=$(sst_get_prefix $ag_json)
    if [[ "$prefix" == "" ]]; then
      sst_barf 'document must have its own subdirectory: %s' $ag_json
    fi

    sst_jq_get_string "$ag_json" .tarname tarname
    if [[ ! "$tarname" ]]; then
      tarname=${html%/*}
      tarname=${tarname##*/}
    fi

    tar_file=$(sst_get_prefix ${prefix%/})$tarname.tar
    tar_file_slug=$(sst_underscore_slug $tar_file)

    sst_jq_get_string_or_null .clean_rule $ag_json clean_rule
    case $clean_rule in
      '' | mostlyclean | clean | distclean | maintainer-clean)
        :
      ;;
      *)
        sst_barf '%s: .clean_rule: invalid value' $ag_json
      ;;
    esac

    sst_jq_get_boolean_or_null .distribute $ag_json distribute

    sst_jq_get_string_or_null .imagesdir $ag_json imagesdir
    if [[ "$imagesdir" == "" ]]; then
      imagesdir=images
    else
      sst_expect_source_path "$imagesdir"
    fi

    sst_am_append <<EOF

#-----------------------------------------------------------------------
# $html
#-----------------------------------------------------------------------

${slug}_child_prerequisites =
${slug}_leaves =

$prefix$imagesdir:
	\$(AM_V_at)\$(MKDIR_P) \$@

${slug}_child_prerequisites += $prefix$imagesdir
EOF

    if [[ "$distribute" == true ]]; then
      sst_am_distribute_file $html
      if [[ "$clean_rule" != "" ]]; then
        sst_warn '%s: ignoring clean_rule because distribute is true' $ag_json
      fi
      clean_rule=maintainer-clean
    elif [[ "$clean_rule" == "" ]]; then
      clean_rule=mostlyclean
    fi

    seen=" "
    for x in $prefix**/*{.ag.json,.ag,.ac,.am,.im.in,.in,.im,}; do
      sst_expect_source_path "$x"

      if [[ $x == $ag_json || $x == $html ]]; then
        continue
      fi

      case $x in
        *.phony.ag.json)
          # TODO: sst_ajh_auto $x
          sst_barf '*.ag.json support is not implemented yet'
          continue
        ;;
        *.phony.ag) sst_ag_include $x; continue ;;
        *.phony.ac) sst_ac_include $x; continue ;;
        *.phony.am) sst_am_include $x; continue ;;
        *.ag.json)
          y=${x%.ag.json}
        ;;
        *.ag)
          y=${x%.ag}
        ;;
        *.ac)
          y=${x%.ac}
        ;;
        *.am)
          y=${x%.am}
        ;;
        *.im.in)
          y=${x%.im.in}
        ;;
        *.in)
          y=${x%.im}
        ;;
        *.im)
          y=${x%.in}
        ;;
        *)
          y=$x
        ;;
      esac

      if [[ "$seen" == *" $y "* ]]; then
        continue
      fi
      seen+="$y "

      # TODO: Only use the leaf prerequisites if "distribute" is true.
      # Otherwise it makes more sense to use the child prerequisites.

      case $x in
        *.ag.json)
          # TODO: sst_ajh_auto $x
          sst_barf '*.ag.json support is not implemented yet'
        ;;
        *.ag)
          sst_ag_include $x
        ;;
        *.ac)
          sst_ac_include $x
          autogen_am_var_append_files ${slug}_leaves $x
        ;;
        *.am)
          sst_am_include $x
          autogen_am_var_append_files ${slug}_leaves $x
        ;;
        *.im.in)
          sst_ac_config_file $y
          autogen_am_var_append_files ${slug}_leaves $x
        ;;
        *.in)
          sst_ac_config_file $y
          autogen_am_var_append_files ${slug}_leaves $x
        ;;
        *.im)
          sst_ac_config_file $y
          autogen_am_var_append_files ${slug}_leaves $x
        ;;
        *)
          autogen_am_var_append_files ${slug}_leaves $x
          sst_am_distribute_file $x
        ;;
      esac

    done

    sst_am_append <<EOF

$html: \$(${slug}_child_prerequisites)
$html: \$(${slug}_leaves)
$html\$(${slug}_disable_wrapper_recipe):
	\$(AM_V_at)rm -f -r \\
	  \$@ \\
	  \$@\$(TSUF)* \\
	  $prefix$imagesdir/diag-* \\
	;
	\$(AM_V_at){ \\
	  flags='-a imagesdir=$imagesdir '; \\
	  \$(SHELL) - \\
	    '\$(srcdir)'/build-aux/echo.sh -q -- \\
	    \$(ASCIIDOCTOR_FLAGS) \\
	    >\$@\$(TSUF) \\
	  || exit \$\$?; \\
	  flags=\$\$flags\`cat \$@\$(TSUF)\` || exit \$\$?; \\
	  rm -f -r \$@\$(TSUF)*; \\
	  \$(MAKE) \\
	    ${slug}_disable_wrapper_recipe=/x \\
	    ASCIIDOCTOR_FLAGS="\$\$flags" \\
	    \$@ \\
	  || exit \$\$?; \\
	}

$html/clean: FORCE
	-rm -f -r \\
	  \$(@D) \\
	  \$(@D)\$(TSUF)* \\
	  $prefix$imagesdir/diag-* \\
	;

$clean_rule-local: $html/clean

$tar_file: $html
	\$(AM_V_at)rm -f -r \$@ \$@\$(TSUF)*
	\$(AM_V_at)\$(MKDIR_P) \$@\$(TSUF)1/$tarname
	\$(AM_V_at)cp $html \$@\$(TSUF)1/$tarname
	@{ \\
	  xs=; \\
	  for ext in \\
	    .css \\
	    .gif \\
	    .jpg \\
	    .js \\
	    .png \\
	    .svg \\
	  ; do \\
	    xs="\$\$xs "\` \\
	      find $prefix. -name "*\$\$ext" -type f \\
	    \` || exit \$\$?; \\
	  done; \\
	  for x in \$\$xs; do \\
	    y=\$\${x#$prefix./}; \\
	    case \$\$y in \\
	      */*) \\
	        d=\$\${y%/*}; \\
	        \$(AM_V_P) && echo \$(MKDIR_P) \\
	          \$@\$(TSUF)1/$tarname/\$\$d \\
	        ; \\
	        \$(MKDIR_P) \\
	          \$@\$(TSUF)1/$tarname/\$\$d \\
	        || exit \$\$?; \\
	      ;; \\
	      *) \\
	        d=.; \\
	      ;; \\
	    esac; \\
	    \$(AM_V_P) && echo cp \\
	      \$\$x \\
	      \$@\$(TSUF)1/$tarname/\$\$d \\
	    ; \\
	    cp \\
	      \$\$x \\
	      \$@\$(TSUF)1/$tarname/\$\$d \\
	    || exit \$\$?; \\
	  done; \\
	}
	\$(AM_V_at)(cd \$@\$(TSUF)1 && \$(TAR) c $tarname) >\$@\$(TSUF)2
	\$(AM_V_at)mv -f \$@\$(TSUF)2 \$@

${tar_file_slug}_leaves = \$(${slug}_leaves)

$tar_file/clean: FORCE
$tar_file/clean: $html/clean
	-rm -f -r \$(@D) \$(@D)\$(TSUF)*

mostlyclean-local: $tar_file/clean

#-----------------------------------------------------------------------
EOF

    # Distribute any images generated by Asciidoctor Diagram.
    if [[ "$distribute" == true ]]; then
      autogen_am_var_append EXTRA_DIST $prefix$imagesdir
    fi

  done

}; readonly -f sst_ajh_asciidoctor_document
