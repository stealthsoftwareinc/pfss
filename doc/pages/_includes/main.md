{%- comment -%}
//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//
{%- endcomment -%}

{%- comment -%}
//
// This file should roughly mirror the doc/readme/main.adoc file. If you
// edit this file, you might need to edit that file as well.
//
{%- endcomment -%}

PFSS is a library for function secret sharing (FSS) developed by Stealth
Software Technologies,&nbsp;Inc.&nbsp;(Stealth).
Our library provides APIs for performing an efficient two-way sharing of
point functions based on the work from Gilboa-Ishai (EUROCRYPT 2014) and
Boyle-Gilboa-Ishai (EUROCRYPT 2015) with optimizations from the
Boyle-Gilboa-Ishai (CCS 2016) result.
We also provide batched APIs to reduce overhead, which is useful for
applications that use FSS for large-scale computations, such as our
recent work of applying FSS to various flavors of Private Set
Intersection.
For the latter work, we provide a reference to Dittmer et al. on
[arXiv](https://arxiv.org/abs/2012.13053)
and
[IACR ePrint](https://eprint.iacr.org/2020/1599).
