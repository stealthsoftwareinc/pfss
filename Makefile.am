##
## For the copyright information for this file, please search up the
## directory tree for the first COPYING file.
##

AM_CPPFLAGS =
AM_CPPFLAGS += -Isrc/c_cpp/include
AM_CPPFLAGS += -I$(srcdir)/src/c_cpp/include
AM_CPPFLAGS += -Isrc/c_cpp/lib
AM_CPPFLAGS += -I$(srcdir)/src/c_cpp/lib
AM_CPPFLAGS += -Isrc/c_cpp/misc
AM_CPPFLAGS += -I$(srcdir)/src/c_cpp/misc

## begin_variables

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = \
  -pthread \
  -std=c11 \
  -Wpedantic \
$(prevent_a_blank_line)

AM_CXXFLAGS = \
  -pthread \
  -Wpedantic \
$(prevent_a_blank_line)

EXTRA_DIST =

MAINTAINERCLEANFILES =

SUFFIXES =

TESTS =

TMPEXT = .tmp

bin_PROGRAMS =

check_PROGRAMS =

include_HEADERS =

lib_LTLIBRARIES =

noinst_HEADERS =

## end_variables

## begin_rules

.PHONY: FORCE

FORCE:

## end_rules

## begin_includes

include $(srcdir)/build-aux/autogen.am
include $(srcdir)/src/c_cpp/bin/pfss-benchmark.am
include $(srcdir)/src/c_cpp/bin/pfss-standard-benchmark.am
include $(srcdir)/src/c_cpp/libpfss_la.am

## end_includes

## begin_includes

include $(srcdir)/build-aux/GATBPS_CONFIG_FILE_RULES.am
include $(srcdir)/build-aux/GATBPS_CONFIG_LATER_RULES.am
include $(srcdir)/build-aux/GATBPS_CP_RULES.am
include $(srcdir)/build-aux/GATBPS_DOCKER_BUILD_RULES.am
include $(srcdir)/build-aux/GATBPS_GIT_ARCHIVE_RULES.am
include $(srcdir)/build-aux/GATBPS_GIT_BUNDLE_RULES.am
include $(srcdir)/build-aux/GATBPS_GIT_CLONE_RULES.am
include $(srcdir)/build-aux/GATBPS_VPATH_SEARCH.am
include $(srcdir)/build-aux/GATBPS_V_PAD.am
include $(srcdir)/build-aux/GATBPS_WGET_RULES.am
include $(srcdir)/build-aux/cp.am
include $(srcdir)/build-aux/docker.am
include $(srcdir)/build-aux/gatbps.am
include $(srcdir)/build-aux/git.am
include $(srcdir)/build-aux/wget.am

## end_includes
