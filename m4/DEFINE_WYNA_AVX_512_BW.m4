dnl
dnl Copyright (C) Stealth Software Technologies, Inc.
dnl
dnl For the complete copyright information, please see the
dnl associated README file.
dnl

AC_DEFUN_ONCE([DEFINE_WYNA_AVX_512_BW], [
GATBPS_CALL_COMMENT([$0]m4_if(m4_eval([$# > 0]), [1], [, $@]))
{ :

  GATBPS_BEFORE([$0], [DEFINE_WYNAS])

  GATBPS_ARG_WYNA(
    [--with-avx-512-bw],
    [
      Use the AVX-512 BW instruction set.
    ],
    [
      Do not use the AVX-512 BW instruction set.
    ],
    [
      Automatically decide whether to use the AVX-512 BW instruction
      set.
    ],
    [
      --with-avx-512-f
      --with-sse
      --with-sse2
    ])

}])
