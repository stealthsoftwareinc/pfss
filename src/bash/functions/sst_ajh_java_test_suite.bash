#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

sst_ajh_java_test_suite() {

  local ag
  local built_jardeps
  local dst
  local java_slug
  local java_target_slug
  local jcl
  local jcl_is_in_jardeps
  local package
  local slug

  sst_ajh_java_library "$@"

  for ag; do

    dst=$(jq -r .dst $ag)

    jq_expect_string .jcl $ag
    jcl=$(jq -r .jcl $ag)
    expect_safe_path "$jcl"
    case $jcl in
      ?*.jar)
      ;;
      *)
        sst_barf '%s: .jcl: expected *.jar: %s' $ag $jcl
      ;;
    esac
    case $jcl in
      */*)
        sst_barf '%s: .jcl: no slashes allowed: %s' $ag $jcl
      ;;
    esac

    jq_expect_strings .jardeps $ag
    jcl_is_in_jardeps=$(jq -r '
      .jardeps | map(sub("^.*/"; "")) | contains(["'$jcl'"])
    ' $ag)
    case $jcl_is_in_jardeps in
      false)
        sst_barf '%s: .jcl must appear in .jardeps' $ag
      ;;
    esac

    package=$(jq -r .package $ag)

    slug=$(jq -r '.slug | select(.)' $ag)
    case $slug in
      "")
        java_target_slug=java-main
        java_slug=java
      ;;
      *)
        java_target_slug=java-$slug
        java_slug=java_$slug
      ;;
    esac

    built_jardeps=$(jq -r '.built_jardeps | select(.) | .[]' $ag)

    sst_ihs <<<"

      check-$java_target_slug: FORCE
      check-$java_target_slug: $dst
      	@{ \\
      	\\
      	  if test -f \$(javadir)/$jcl; then \\
      	    jar=\$(javadir)/$jcl; \\
      	  elif test -f /usr/local/share/java/$jcl; then \\
      	    jar=/usr/local/share/java/$jcl; \\
      	  else \\
      	    jar=/usr/share/java/$jcl; \\
      	  fi; \\
      	  readonly jar; \\
      	\\
      	  method='\$(JAVA_TEST_METHOD)'; \\
      	  readonly method; \\
      	\\
      	  reports_dir='\$(JAVA_TEST_REPORTS_DIR)'; \\
      	  case \$\$reports_dir in \\
      	    '') \\
      	      reports_dir=java-test-reports; \\
      	    ;; \\
      	  esac; \\
      	  readonly reports_dir; \\
      	\\
      	  case \$\$method in \\
      	    '') \\
      	      \$(AM_V_P) && \$(SHELL) - \\
      	        '\$(srcdir)'/build-aux/echo.sh -q -- \\
      	        \$(JAVA) \\
      	          -jar \$\$jar \\
      	          --classpath='$dst\$(CLASSPATH_SEPARATOR)\$(${java_slug}_CLASSPATH)' \\
      	          --details=flat \\
      	          --details-theme=ascii \\
      	          --disable-ansi-colors \\
      	          --disable-banner \\
      	          --reports-dir=\"\$\$reports_dir\" \\
      	          --select-package=$package \\
      	          --include-classname='^.*\$\$' \\
      	      ; \\
      	      \$(JAVA) \\
      	        -jar \$\$jar \\
      	        --classpath='$dst\$(CLASSPATH_SEPARATOR)\$(${java_slug}_CLASSPATH)' \\
      	        --details=flat \\
      	        --details-theme=ascii \\
      	        --disable-ansi-colors \\
      	        --disable-banner \\
      	        --reports-dir=\"\$\$reports_dir\" \\
      	        --select-package=$package \\
      	        --include-classname='^.*\$\$' \\
      	      || exit \$\$?; \\
      	    ;; \\
      	    *) \\
      	      x=$package.\$\${method%.*}; \\
      	      y=\$\${method##*.}; \\
      	      \$(AM_V_P) && \$(SHELL) - \\
      	        '\$(srcdir)'/build-aux/echo.sh -q -- \\
      	        \$(JAVA) \\
      	          -jar \$\$jar \\
      	          --classpath='$dst\$(CLASSPATH_SEPARATOR)\$(${java_slug}_CLASSPATH)' \\
      	          --details=flat \\
      	          --details-theme=ascii \\
      	          --disable-ansi-colors \\
      	          --disable-banner \\
      	          --reports-dir=\"\$\$reports_dir\" \\
      	          --select-method=\"\$\$x#\$\$y\" \\
      	      ; \\
      	      \$(JAVA) \\
      	        -jar \$\$jar \\
      	        --classpath='$dst\$(CLASSPATH_SEPARATOR)\$(${java_slug}_CLASSPATH)' \\
      	        --details=flat \\
      	        --details-theme=ascii \\
      	        --disable-ansi-colors \\
      	        --disable-banner \\
      	        --reports-dir=\"\$\$reports_dir\" \\
      	        --select-method=\"\$\$x#\$\$y\" \\
      	      || exit \$\$?; \\
      	    ;; \\
      	  esac; \\
      	\\
      	}

      check-$java_target_slug/clean: FORCE
      	@{ \\
      	\\
      	  reports_dir='\$(JAVA_TEST_REPORTS_DIR)'; \\
      	  case \$\$reports_dir in \\
      	    '') \\
      	      reports_dir=java-test-reports; \\
      	    ;; \\
      	  esac; \\
      	  readonly reports_dir; \\
      	\\
      	  \$(AM_V_P) && \$(SHELL) - \\
      	    '\$(srcdir)'/build-aux/echo.sh -q -- \\
      	    rm -f -r \\
      	      \"\$\$reports_dir\" \\
      	  ; \\
      	  rm -f -r \\
      	    \"\$\$reports_dir\" \\
      	  || :; \\
      	\\
      	}

      mostlyclean-local: check-$java_target_slug/clean

      check-java: FORCE
      check-java: check-$java_target_slug

    " | sst_am_append

  done

}; readonly -f sst_ajh_java_test_suite
