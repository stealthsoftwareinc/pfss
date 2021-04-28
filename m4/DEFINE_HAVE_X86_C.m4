dnl
dnl For the copyright information for this file, please search up the
dnl directory tree for the first COPYING file.
dnl

AC_DEFUN_ONCE([DEFINE_HAVE_X86_C], [[{ :

]GATBPS_SOFT_REQUIRE([[DEFINE_WITH_X86]])[

]m4_define(
  [HAVE_X86_C_PROLOGUE],
  [[[
  ]]])[

]m4_define(
  [HAVE_X86_C_BODY],
  [[[

    /*
     * See <https://sourceforge.net/p/predef/wiki/Architectures/>.
     */

    #if !(0 \
      || defined(_M_AMD64) \
      || defined(_M_I86) \
      || defined(_M_IX86) \
      || defined(_M_X64) \
      || defined(_X86_) \
      || defined(__386) \
      || defined(__I86__) \
      || defined(__IA32__) \
      || defined(__INTEL__) \
      || defined(__THW_INTEL__) \
      || defined(__X86__) \
      || defined(__amd64) \
      || defined(__amd64__) \
      || defined(__i386) \
      || defined(__i386__) \
      || defined(__i486__) \
      || defined(__i586__) \
      || defined(__i686__) \
      || defined(__x86_64) \
      || defined(__x86_64__) \
      || defined(i386) \
    )

      #include <cause_a_compilation_error>

    #endif

  ]]])[

]AC_LANG_PUSH([C])[
]GATBPS_CHECK_COMPILE(
  [x86 (C)],
  [HAVE_X86_C],
  HAVE_X86_C_PROLOGUE,
  HAVE_X86_C_BODY,
  [
    ]GATBPS_SOFT_VAR_DEP([[WITH_X86]])[
  ])[
]AC_LANG_POP([C])[

}]])
