/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#ifndef PFSS_H
#define PFSS_H

#include <assert.h>
#include <limits.h>
#include <pfss/config.h>
#include <pfss/switch_db_rb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if PFSS_HAVE_SSE2
#include <emmintrin.h>
#endif

#if PFSS_HAVE_AES_NI
#include <wmmintrin.h>
#endif

/*----------------------------------------------------------------------
// PFSS_EXTERN_C
//--------------------------------------------------------------------*/

#ifdef __cplusplus
#define PFSS_EXTERN_C extern "C"
#else
#define PFSS_EXTERN_C extern
#endif

/*----------------------------------------------------------------------
// PFSS_PUBLIC_C_FUNCTION
//--------------------------------------------------------------------*/

#ifndef PFSS_PUBLIC_C_FUNCTION

#if !defined(_WIN32) || !defined(DLL_EXPORT)
#define PFSS_PUBLIC_C_FUNCTION PFSS_EXTERN_C
#elif defined(PFSS_BUILD)
#define PFSS_PUBLIC_C_FUNCTION PFSS_EXTERN_C __declspec(dllexport)
#else
#define PFSS_PUBLIC_C_FUNCTION PFSS_EXTERN_C __declspec(dllimport)
#endif

#endif /* ifndef PFSS_PUBLIC_C_FUNCTION */

/*----------------------------------------------------------------------
// PFSS_PUBLIC_C_OBJECT
//--------------------------------------------------------------------*/

#ifndef PFSS_PUBLIC_C_OBJECT

#if !defined(_WIN32) || !defined(DLL_EXPORT)
#define PFSS_PUBLIC_C_OBJECT PFSS_EXTERN_C
#elif defined(PFSS_BUILD)
#define PFSS_PUBLIC_C_OBJECT PFSS_EXTERN_C __declspec(dllexport)
#else
#define PFSS_PUBLIC_C_OBJECT PFSS_EXTERN_C __declspec(dllimport)
#endif

#endif /* ifndef PFSS_PUBLIC_C_OBJECT */

/*----------------------------------------------------------------------
// PFSS_PUBLIC_CPP_FUNCTION
//--------------------------------------------------------------------*/

#ifndef PFSS_PUBLIC_CPP_FUNCTION

#if !defined(_WIN32) || !defined(DLL_EXPORT)
#define PFSS_PUBLIC_CPP_FUNCTION extern
#elif defined(PFSS_BUILD)
#define PFSS_PUBLIC_CPP_FUNCTION extern __declspec(dllexport)
#else
#define PFSS_PUBLIC_CPP_FUNCTION extern __declspec(dllimport)
#endif

#endif /* ifndef PFSS_PUBLIC_CPP_FUNCTION */

/*----------------------------------------------------------------------
// PFSS_PUBLIC_CPP_OBJECT
//--------------------------------------------------------------------*/

#ifndef PFSS_PUBLIC_CPP_OBJECT

#if !defined(_WIN32) || !defined(DLL_EXPORT)
#define PFSS_PUBLIC_CPP_OBJECT extern
#elif defined(PFSS_BUILD)
#define PFSS_PUBLIC_CPP_OBJECT extern __declspec(dllexport)
#else
#define PFSS_PUBLIC_CPP_OBJECT extern __declspec(dllimport)
#endif

#endif /* ifndef PFSS_PUBLIC_CPP_OBJECT */

/*----------------------------------------------------------------------
// PFSS_PUBLIC_CPP_CLASS
//--------------------------------------------------------------------*/

#ifndef PFSS_PUBLIC_CPP_CLASS

#if !defined(_WIN32) || !defined(DLL_EXPORT)
#define PFSS_PUBLIC_CPP_CLASS
#elif defined(PFSS_BUILD)
#define PFSS_PUBLIC_CPP_CLASS __declspec(dllexport)
#else
#define PFSS_PUBLIC_CPP_CLASS __declspec(dllimport)
#endif

#endif /* ifndef PFSS_PUBLIC_CPP_CLASS */

/*----------------------------------------------------------------------
// pfss_status
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_status[]
[[c_api_pfss_status]]
== The `pfss_status` type

[source,c]
----
typedef uint32_t pfss_status;
----

The `pfss_status` type represents a status code of a function call.
Most PFSS functions return a `pfss_status` code.

`pfss_status` is guaranteed to be an alias of `uint32_t`.

Each status code is defined as a macro that expands to an integer
constant expression that can be used in both normal code and in
preprocessor arithmetic.
In normal code, the type of the expression is `pfss_status`.

The status codes and their meanings are listed below.
Note that some meanings are more descriptive than others.
Functions generally return the most descriptive status code they can.

* 0 (0x0) `PFSS_OK` +
The function succeeded.

* 1 (0x1) `PFSS_UNKNOWN_ERROR` +
The function failed.

* 2 (0x2) `PFSS_INVALID_ARGUMENT` +
The function failed because of an invalid argument.

* 3 (0x3) `PFSS_NULL_POINTER` +
The function failed because of an invalid argument.
In particular, an argument was incorrectly a null pointer.

* 4 (0x4) `PFSS_INVALID_DOMAIN` +

* 5 (0x5) `PFSS_INVALID_RANGE` +

* 6 (0x6) `PFSS_UNSUPPORTED_DOMAIN_AND_RANGE` +

* 7 (0x7) `PFSS_DOMAIN_OVERFLOW` +

* 8 (0x8) `PFSS_RANGE_OVERFLOW` +

* 9 (0x9) `PFSS_MALFORMED_KEY` +

end::c_api_pfss_status[]
*/

typedef uint32_t pfss_status;

/*
 * The ((T) + n) syntax allows an integer constant with type T to work
 * in both normal code and in preprocessor arithmetic. In normal code,
 * it is interpreted as the expression +n (unary plus) cast to type T.
 * In preprocessor arithmetic, T is not defined as a macro, so it is
 * replaced with 0, resulting in ((0) + n) (binary plus). See N1256
 * 6.10.1p4 or N3337 16.1p4. Here is the relevant text from N1256:
 *
 *       After all replacements due to macro expansion and the
 *       defined unary operator have been performed, all remaining
 *       identifiers (including those lexically identical to
 *       keywords) are replaced with the pp-number 0, and then
 *       each preprocessing token is converted into a token.
 */

#define PFSS_OK ((pfss_status) + 0)
#define PFSS_UNKNOWN_ERROR ((pfss_status) + 1)
#define PFSS_INVALID_ARGUMENT ((pfss_status) + 2)
#define PFSS_NULL_POINTER ((pfss_status) + 3)
#define PFSS_INVALID_DOMAIN ((pfss_status) + 4)
#define PFSS_INVALID_RANGE ((pfss_status) + 5)
#define PFSS_UNSUPPORTED_DOMAIN_AND_RANGE ((pfss_status) + 6)
#define PFSS_DOMAIN_OVERFLOW ((pfss_status) + 7)
#define PFSS_RANGE_OVERFLOW ((pfss_status) + 8)
#define PFSS_MALFORMED_KEY ((pfss_status) + 9)

/*----------------------------------------------------------------------
// pfss_get_status_name
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
char const * pfss_get_status_name(pfss_status status);

/*----------------------------------------------------------------------
// Logging
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
FILE * pfss_set_log_file(FILE *);

PFSS_PUBLIC_C_FUNCTION
FILE * pfss_get_log_file(void);

/*----------------------------------------------------------------------
// pfss_endianness
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_endianness[]
[[c_api_pfss_endianness]]
== The `pfss_endianness` type

[source,c]
----
typedef uint32_t pfss_endianness;
----

The `pfss_endianness` type describes the byte order of an integer value.

The endianness constants and their meanings are listed below.

* 0 (0x0) `PFSS_NATIVE_ENDIAN` +
The native byte order of the system.

* 1 (0x1) `PFSS_LITTLE_ENDIAN` +
Little endian byte order.

* 2 (0x2) `PFSS_BIG_ENDIAN` +
Big endian byte order.

end::c_api_pfss_endianness[]
*/

typedef uint32_t pfss_endianness;

/*
 * The ((T) + n) syntax used for the constants is explained in the
 * pfss_status documentation earlier in this file.
 */

#define PFSS_NATIVE_ENDIAN ((pfss_endianness) + 0)
#define PFSS_LITTLE_ENDIAN ((pfss_endianness) + 1)
#define PFSS_BIG_ENDIAN ((pfss_endianness) + 2)

/*----------------------------------------------------------------------
// pfss_gen_sizes
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_gen_sizes[]
[[c_api_pfss_gen_sizes]]
== The `pfss_gen_sizes` function

[source,c]
----
pfss_status pfss_gen_sizes(
  uint32_t   domain_bits,
  uint32_t   range_bits,
  uint32_t * key_blob_size,
  uint32_t * rand_buf_size
);
----

The `pfss_gen_sizes` function determines how big each of the key blobs
generated by the `pfss_gen` function will be for the given `domain_bits`
and `range_bits` values, and how many random bytes will be needed to
generate them.

end::c_api_pfss_gen_sizes[]
*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_gen_sizes(
    uint32_t domain_bits,
    uint32_t range_bits,
    uint32_t * key_blob_size,
    uint32_t * rand_buf_size);

/*----------------------------------------------------------------------
// pfss_gen
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_gen(
    uint32_t domain_bits,
    uint32_t range_bits,
    uint8_t const * alpha,
    uint32_t alpha_size,
    pfss_endianness alpha_endianness,
    uint8_t const * beta,
    uint32_t beta_size,
    pfss_endianness beta_endianness,
    uint8_t * key1_blob,
    uint8_t * key2_blob,
    uint8_t const * rand_buf);

/*----------------------------------------------------------------------
// pfss_map_gen
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_map_gen[]
[[c_api_pfss_map_gen]]
== The `pfss_map_gen` function

[source,c]
----
pfss_status pfss_map_gen(
  uint32_t        domain_bits,
  uint32_t        range_bits,
  uint8_t const * alphas,
  uint32_t        alphas_count,
  uint32_t        alpha_size,
  pfss_endianness alpha_endianness,
  uint8_t const * betas,
  uint32_t        beta_size,
  pfss_endianness beta_endianness,
  uint8_t *       key1_blobs,
  uint8_t *       key2_blobs,
  uint8_t const * rand_bufs
);
----

The `pfss_map_gen` function creates FSS key pairs for each of the
`alphas_count` elements of `alphas` and `betas`, storing the key pairs
inside `key1_blobs` and `key2_blobs`.
Each element of `alphas` should be an unsigned integer that consists of
`alpha_size` bytes in `alpha_endianness` byte order.
Each element of `betas` should be an unsigned integer that consists of
`beta_size` bytes in `beta_endianness` byte order.
Each element in alphas should be less than 2^domain_bits and each
element of betas should be less than 2^range_bits. As long as the
values of each element in alphas and betas are in range, alpha_size and
beta_size can be arbitrarily large or small, but they must not be zero.
`key1_blobs` and `key2_blobs` should be containers consisting of
`alphas_count` * `key_blob_size` bytes.
Similarly, `rand_bufs` should be a container consisting of
`alphas_count` * `rand_buf_size` bytes.
The function returns `PFSS_OK` upon success, or another status code upon
failure.

end::c_api_pfss_map_gen[]
*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_map_gen(
    uint32_t domain_bits,
    uint32_t range_bits,
    uint8_t const * alphas,
    uint32_t alphas_count,
    uint32_t alpha_size,
    pfss_endianness alpha_endianness,
    uint8_t const * betas,
    uint32_t beta_size,
    pfss_endianness beta_endianness,
    uint8_t * key1_blobs,
    uint8_t * key2_blobs,
    uint8_t const * rand_bufs);

/*----------------------------------------------------------------------
// pfss_key
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_key[]
[[c_api_pfss_key]]
== The `pfss_key` type

[source,c]
----
typedef struct pfss_key pfss_key;
----

The `pfss_key` type represents a key that has been parsed from a key
blob.

end::c_api_pfss_key[]
*/

typedef struct pfss_key pfss_key;

/*----------------------------------------------------------------------
// pfss_parse_key
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_parse_key[]
[[c_api_pfss_parse_key]]
== The `pfss_parse_key` function

[source,c]
----
pfss_status pfss_parse_key(
  pfss_key * *    p_key,
  uint8_t const * key_blob,
  uint32_t        key_blob_size
);
----

The `pfss_parse_key` function parses a key blob.

end::c_api_pfss_parse_key[]
*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_parse_key(
    pfss_key ** p_key,
    uint8_t const * key_blob,
    uint32_t key_blob_size);

/*----------------------------------------------------------------------
// pfss_destroy_key
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_destroy_key[]
[[c_api_pfss_destroy_key]]
== The `pfss_destroy_key` function

[source,c]
----
pfss_status pfss_destroy_key(
  pfss_key * key
);
----

The `pfss_destroy_key` function destroys a key.

end::c_api_pfss_destroy_key[]
*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_destroy_key(pfss_key * key);

/*----------------------------------------------------------------------
// pfss_get_domain_bits
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status
pfss_get_domain_bits(pfss_key const * key, uint32_t * domain_bits);

/*----------------------------------------------------------------------
// pfss_get_range_bits
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status
pfss_get_range_bits(pfss_key const * key, uint32_t * range_bits);

/*----------------------------------------------------------------------
// pfss_eval
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval(
    pfss_key const * key,
    uint8_t const * x,
    uint32_t x_size,
    pfss_endianness x_endianness,
    uint8_t * y,
    uint32_t y_size,
    pfss_endianness y_endianness);

/*----------------------------------------------------------------------
// PFSS_DEFINE_DIRECT_EVAL
//--------------------------------------------------------------------*/

/*
tag::c_api_PFSS_DEFINE_DIRECT_EVAL[]
[[c_api_PFSS_DEFINE_DIRECT_EVAL]]
== The `PFSS_DEFINE_DIRECT_EVAL` macro

[source,c,subs="replacements"]
----
#define PFSS_DEFINE_DIRECT_EVAL(f, domain_bits, range_type) \
  static range_type f(void const * k, void const * x) { ... }
----

The `PFSS_DEFINE_DIRECT_EVAL` macro defines a self-contained function
`f` that evaluates an unparsed key blob `k` at a domain value `x`.
`f` can be any name of your choice.
`domain_bits` should be an integer constant expression with type `int`
whose value is between 1 and 128 inclusive.
`range_type` should be either `uint8_t`, `uint16_t`, `uint32_t`, or
`uint64_t`.

When calling `f`, `k` should be a key blob that was output by the
`pfss_gen` function with the same number of domain bits and range bits
as specified by `domain_bits` and `range_type`, and `x` should be an
unsigned integer that consists of
stem:[\lceil\mathtt{domain\_bits}/8\rceil] bytes in little endian byte
order.
Any bits above the least significant `domain_bits` bits of `x` are
ignored.
The return value is the result of the evaluation.

The code of `f` is valid as both C and {cpp}.
However, `f` currently requires AES-NI support via the `-maes` compiler
option.
This option is normally handled by the PFSS build system, but since
you'll be compiling `f` yourself, you'll need to provide this option
yourself.
In the future, it may be generalized to work on other systems.
////
However, depending on how PFSS was built, `f` may be expecting special
CPU instructions to be available, which may require special compiler
options.
These options are normally handled by the PFSS build system, but since
you'll be compiling `f` yourself, you'll need to provide these options
yourself.
For x86 family CPUs, the `-maes` option may be necessary.
For ARM family CPUs, any `-march=...` and `-mcpu=...` options that were
used to build PFSS should be used.
You can also search for "```CPPFLAGS =```", "```CFLAGS =```", or
"```CXXFLAGS =```" in the `Makefile` created by the `./configure` step
of the PFSS build system to see the exact options that it chose to use
for the preprocessor, the C compiler, and the {cpp} compiler.
See the <<hardware_acceleration>> section for more information about
these special compiler options.
////

.{blank}
====
The following code uses the `PFSS_DEFINE_DIRECT_EVAL` macro to define a
self-contained function
`static uint16_t my_eval(void const * k, void const * x) { ... }` that
evaluates an unparsed key blob `k` with 128 domain bits and 16 range
bits at a domain value `x`:

[source,c]
----
#include <pfss.h>
#include <stdint.h>
PFSS_DEFINE_DIRECT_EVAL(my_eval, 128, uint16_t)
----
====

end::c_api_PFSS_DEFINE_DIRECT_EVAL[]
*/

#if PFSS_HAVE_AES_NI

#define PFSS_DEFINE_DIRECT_EVAL(f, domain_bits, range_type) \
  static range_type f(void const * const k, void const * const x) { \
\
    enum { \
      db = (domain_bits), \
      range_bytes = sizeof(range_type), \
      range_bits = range_bytes * 8, \
      n_minus_v_2 = \
          range_bits == 8 ? \
              5 : \
              range_bits == 16 ? 4 : range_bits == 32 ? 3 : 2, \
      n_minus_v = db < n_minus_v_2 ? db : n_minus_v_2, \
      v = db - n_minus_v, \
      v_bytes = v / 8 + (v % 8 != 0) \
    }; \
\
    uint8_t const * const k_base = (uint8_t const *)k; \
    uint8_t const * k_block; \
    uint8_t const * k_party; \
    uint8_t const * k_tcw_0; \
    uint8_t const * k_tcw_1; \
    uint8_t const * k_cw_last; \
    int party; \
    __m128i tcw_0; \
    __m128i tcw_1; \
    uint64_t cw_last[2] = {0, 0}; \
\
    uint8_t const * const x_base = (uint8_t const *)x; \
    __m128i x_val; \
    __m128i tcw_x_val; \
    uint8_t const * const tcw_x = (uint8_t const *)&tcw_x_val; \
\
    __m128i rkeys[11]; \
    __m128i msb[2]; \
\
    __m128i scw[2]; \
    __m128i s; \
    __m128i pi_s; \
    int t; \
    int i; \
    int cw_last_i; \
    int cw_last_i_r; \
    range_type y[2]; \
\
    assert(db >= 1); \
    assert(db <= 128); \
    assert(range_bits >= 1); \
    assert(range_bits <= 64); \
    assert(range_bits % 8 == 0); \
\
    assert(k != NULL); \
    assert(x != NULL); \
\
    assert(k_base[0] == 0); \
    assert(k_base[1] == db); \
    assert(k_base[2] == range_bits); \
\
    k_block = k_base + 3; \
    k_party = k_block + 16 + v * 16; \
    k_tcw_0 = k_party + 1; \
    k_tcw_1 = k_tcw_0 + v_bytes; \
    k_cw_last = k_tcw_1 + v_bytes; \
\
    party = k_party[0] != 0; \
\
    tcw_0 = _mm_set_epi8(v > 120 ? k_tcw_0[15] : 0, \
                         v > 112 ? k_tcw_0[14] : 0, \
                         v > 104 ? k_tcw_0[13] : 0, \
                         v > 96 ? k_tcw_0[12] : 0, \
                         v > 88 ? k_tcw_0[11] : 0, \
                         v > 80 ? k_tcw_0[10] : 0, \
                         v > 72 ? k_tcw_0[9] : 0, \
                         v > 64 ? k_tcw_0[8] : 0, \
                         v > 56 ? k_tcw_0[7] : 0, \
                         v > 48 ? k_tcw_0[6] : 0, \
                         v > 40 ? k_tcw_0[5] : 0, \
                         v > 32 ? k_tcw_0[4] : 0, \
                         v > 24 ? k_tcw_0[3] : 0, \
                         v > 16 ? k_tcw_0[2] : 0, \
                         v > 8 ? k_tcw_0[1] : 0, \
                         v > 0 ? k_tcw_0[0] : 0); \
    if (db <= 64) { \
      tcw_0 = _mm_slli_epi64(tcw_0, n_minus_v); \
    } else { \
      tcw_0 = _mm_or_si128( \
          _mm_slli_epi64(tcw_0, n_minus_v), \
          _mm_srli_epi64(_mm_slli_si128(tcw_0, 8), 64 - n_minus_v)); \
    } \
\
    tcw_1 = _mm_set_epi8(v > 120 ? k_tcw_1[15] : 0, \
                         v > 112 ? k_tcw_1[14] : 0, \
                         v > 104 ? k_tcw_1[13] : 0, \
                         v > 96 ? k_tcw_1[12] : 0, \
                         v > 88 ? k_tcw_1[11] : 0, \
                         v > 80 ? k_tcw_1[10] : 0, \
                         v > 72 ? k_tcw_1[9] : 0, \
                         v > 64 ? k_tcw_1[8] : 0, \
                         v > 56 ? k_tcw_1[7] : 0, \
                         v > 48 ? k_tcw_1[6] : 0, \
                         v > 40 ? k_tcw_1[5] : 0, \
                         v > 32 ? k_tcw_1[4] : 0, \
                         v > 24 ? k_tcw_1[3] : 0, \
                         v > 16 ? k_tcw_1[2] : 0, \
                         v > 8 ? k_tcw_1[1] : 0, \
                         v > 0 ? k_tcw_1[0] : 0); \
    if (db <= 64) { \
      tcw_1 = _mm_slli_epi64(tcw_1, n_minus_v); \
    } else { \
      tcw_1 = _mm_or_si128( \
          _mm_slli_epi64(tcw_1, n_minus_v), \
          _mm_srli_epi64(_mm_slli_si128(tcw_1, 8), 64 - n_minus_v)); \
    } \
\
    x_val = _mm_set_epi8(db > 120 ? x_base[15] : 0, \
                         db > 112 ? x_base[14] : 0, \
                         db > 104 ? x_base[13] : 0, \
                         db > 96 ? x_base[12] : 0, \
                         db > 88 ? x_base[11] : 0, \
                         db > 80 ? x_base[10] : 0, \
                         db > 72 ? x_base[9] : 0, \
                         db > 64 ? x_base[8] : 0, \
                         db > 56 ? x_base[7] : 0, \
                         db > 48 ? x_base[6] : 0, \
                         db > 40 ? x_base[5] : 0, \
                         db > 32 ? x_base[4] : 0, \
                         db > 24 ? x_base[3] : 0, \
                         db > 16 ? x_base[2] : 0, \
                         db > 8 ? x_base[1] : 0, \
                         db > 0 ? x_base[0] : 0); \
\
    tcw_x_val = _mm_xor_si128(_mm_andnot_si128(x_val, tcw_0), \
                              _mm_and_si128(x_val, tcw_1)); \
\
    rkeys[0] = _mm_set_epi64x(0xF71DBCACE1F3D24B, 0xB0FF11DDA4368097); \
    rkeys[1] = _mm_set_epi64x(0x934F5BC96452E765, 0x85A1352E355E24F3); \
    rkeys[2] = _mm_set_epi64x(0x9A3E294A09717283, 0x6D2395E6E882A0C8); \
    rkeys[3] = _mm_set_epi64x(0xC056DC465A68F50C, 0x5319878F3E3A1269); \
    rkeys[4] = _mm_set_epi64x(0xADA70D226DF1D164, 0x379924686480A3E7); \
    rkeys[5] = _mm_set_epi64x(0x00DA070EAD7D0A2C, 0xC08CDB48F715FF20); \
    rkeys[6] = _mm_set_epi64x(0x315D7EAF318779A1, 0x9CFA738D5C76A8C5); \
    rkeys[7] = _mm_set_epi64x(0xB99190F588CCEE5A, 0xB94B97FB25B1E476); \
    rkeys[8] = _mm_set_epi64x(0x4BF18CC2F2601C37, 0x7AACF26DC3E76596); \
    rkeys[9] = _mm_set_epi64x(0x2569A6716E982AB3, 0x9CF83684E654C4E9); \
    rkeys[10] = \
        _mm_set_epi64x(0x926287BDB70B21CC, 0xD9930B7F456B3DFB); \
\
    msb[0] = _mm_setzero_si128(); \
    msb[1] = _mm_set_epi32(INT_MIN, 0, 0, 0); \
\
    scw[0] = _mm_setzero_si128(); \
    s = _mm_loadu_si128((__m128i const *)k_block); \
    t = party; \
    for (i = 0; i != v; ++i) { \
      int const j = db - 1 - i; \
      scw[1] = _mm_loadu_si128((__m128i const *)(k_block += 16)); \
      scw[1] = \
          _mm_xor_si128(scw[1], msb[(tcw_x[j / 8] >> (j % 8)) & 1]); \
      s = _mm_xor_si128(s, msb[(x_base[j / 8] >> (j % 8)) & 1]); \
      pi_s = _mm_xor_si128(s, rkeys[0]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[1]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[2]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[3]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[4]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[5]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[6]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[7]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[8]); \
      pi_s = _mm_aesenc_si128(pi_s, rkeys[9]); \
      pi_s = _mm_aesenclast_si128(pi_s, rkeys[10]); \
      s = _mm_xor_si128(s, pi_s); \
      s = _mm_xor_si128(s, scw[t]); \
      t = !!(_mm_movemask_epi8(s) & 32768); \
      s = _mm_and_si128(s, _mm_set_epi32(INT_MAX, -1, -1, -1)); \
    } \
    cw_last_i = 0; \
    for (i = v; i != db; ++i) { \
      int const j = db - 1 - i; \
      cw_last_i <<= 1; \
      cw_last_i |= (x_base[j / 8] >> (j % 8)) & 1; \
    } \
    if (cw_last_i < 128 / range_bits) { \
      cw_last_i_r = cw_last_i; \
    } else { \
      cw_last_i_r = cw_last_i - 128 / range_bits; \
      s = _mm_xor_si128(s, msb[1]); \
    } \
    pi_s = _mm_xor_si128(s, rkeys[0]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[1]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[2]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[3]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[4]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[5]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[6]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[7]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[8]); \
    pi_s = _mm_aesenc_si128(pi_s, rkeys[9]); \
    pi_s = _mm_aesenclast_si128(pi_s, rkeys[10]); \
    s = _mm_xor_si128(s, pi_s); \
    memcpy(&y[0], \
           (uint8_t const *)&s + cw_last_i_r * range_bytes, \
           range_bytes); \
    memcpy(&cw_last[1], \
           k_cw_last + cw_last_i * range_bytes, \
           range_bytes); \
    y[0] += cw_last[t] + 0U; \
    y[1] = -y[0]; \
    return y[party]; \
  }

#else

#define PFSS_DEFINE_DIRECT_EVAL(f, domain_bits, range_type) \
  static int PFSS_DEFINE_DIRECT_EVAL_is_not_available[-1];

#endif

/*----------------------------------------------------------------------
// pfss_reduce_sum
//--------------------------------------------------------------------*/

/*
tag::c_api_pfss_reduce_sum[]
[[c_api_pfss_reduce_sum]]
== The `pfss_reduce_sum` function

[source,c]
----
pfss_status pfss_reduce_sum(
  uint32_t        range_bits,
  uint8_t const * ys,
  uint32_t        ys_count,
  uint32_t        y_size,
  pfss_endianness y_endianness,
  uint8_t *       z,
  uint32_t        z_size,
  pfss_endianness z_endianness
);
----

The `pfss_reduce_sum` function sets `x` to the sum of the `ys_count`
elements of `ys`.
Each element of `ys` should be an unsigned integer that consists of
`y_size` bytes in `y_endianness` byte order.
The sum is taken modulo `2^range_bits^` and output as an unsigned
integer that consists of `z_size` bytes in `z_endianness` byte order.
The function returns `PFSS_OK` upon success, or another status code upon
failure.

end::c_api_pfss_reduce_sum[]
*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_reduce_sum(
    uint32_t range_bits,
    uint8_t const * ys,
    uint32_t ys_count,
    uint32_t y_size,
    pfss_endianness y_endianness,
    uint8_t * z,
    uint32_t z_size,
    pfss_endianness z_endianness);

/*----------------------------------------------------------------------
// pfss_map_eval
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_map_eval(
    pfss_key const * key,
    uint8_t const * xs,
    uint32_t xs_count,
    uint32_t x_size,
    pfss_endianness x_endianness,
    uint8_t * ys,
    uint32_t y_size,
    pfss_endianness y_endianness);

/*----------------------------------------------------------------------
// pfss_map_eval_reduce_sum
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_map_eval_reduce_sum(
    pfss_key const * key,
    uint8_t const * xs,
    uint32_t xs_count,
    uint32_t x_size,
    pfss_endianness x_endianness,
    uint8_t * y,
    uint32_t y_size,
    pfss_endianness y_endianness);

/*----------------------------------------------------------------------
// pfss_eval_all
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval_all(
    pfss_key const * key,
    uint8_t const * xp,
    uint32_t xp_bits,
    pfss_endianness xp_endianness,
    uint8_t * ys,
    uint32_t y_size,
    pfss_endianness y_endianness);

/*----------------------------------------------------------------------
// pfss_eval_all_sum
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval_all_sum(pfss_key const * const * keys,
                              uint32_t keys_count,
                              uint8_t * ys,
                              uint32_t y_size,
                              pfss_endianness y_endianness,
                              uint32_t thread_count);

/*----------------------------------------------------------------------
// pfss_eval_all_dot
//--------------------------------------------------------------------*/

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval_all_dot(pfss_key const * const * keys,
                              uint32_t keys_count,
                              uint8_t const * ys,
                              uint32_t y_size,
                              pfss_endianness y_endianness,
                              uint8_t * zs,
                              uint32_t z_size,
                              pfss_endianness z_endianness,
                              uint32_t thread_count);

/*--------------------------------------------------------------------*/

#endif /* PFSS_H */
