#
# For the copyright information for this file, please search up the
# directory tree for the first COPYING file.
#

#
# This script converts switch_db_rb.cfg into switch_db_rb.h.
#

{
  sub(/#.*/, "")
  sub(/^[ 	]+/, "")
  sub(/[ 	]+$/, "")
  if ($0 == "") {
  } else if ($0 ~ /^[0-9]+[ 	]+[0-9]+$/) {
    db = $1
    rb = $2
    sub(/^0+/, "", db)
    sub(/^0+/, "", rb)
    if (db == "") {
      system("echo " FILENAME ":" NR ": domain_bits must be at least 1 >&2")
      exit 1
    }
    if (rb == "") {
      system("echo " FILENAME ":" NR ": range_bits must be at least 1 >&2")
      exit 1
    }
    if (!seen[db, rb]) {
      rb_lists[db] = rb_lists[db] " " rb
      seen[db, rb] = 1
    }
  } else {
    system("echo " FILENAME ":" NR ": line is malformed >&2")
    exit 1
  }
}

END {
  print "//"
  print "// For the copyright information for this file, please search up the"
  print "// directory tree for the first COPYING file."
  print "//"
  print ""
  print "#ifndef PFSS_SWITCH_DB_RB_H"
  print "#define PFSS_SWITCH_DB_RB_H"
  print ""
  print "#define PFSS_SWITCH_DB_RB( \\"
  print "    domain_bits, range_bits, template_code, standard_code) \\"
  print "  do { \\"
  print "    switch ((domain_bits)) { \\"
  for (db in rb_lists) {
    print "      case " db ": switch ((range_bits)) { \\"
    split(rb_lists[db], rb_list)
    for (i in rb_list) {
      rb = rb_list[i]
      print "        case " rb ": template_code(" db ", " rb "); break; \\"
    }
    print "        default: standard_code(); break; \\"
    print "      } break; \\"
  }
  print "      default: switch ((range_bits)) { \\"
  print "        default: standard_code(); break; \\"
  print "      } break; \\"
  print "    } \\"
  print "  } while (0)"
  print ""
  print "#endif // PFSS_SWITCH_DB_RB_H"
}
