/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

// Include first to test independence.
#include <pfss.h>
// Include twice to test idempotence.
#include <pfss.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <future>
#include <limits>
#include <memory>
#include <pfss/bgi1.h>
#include <pfss/common.h>
#include <pfss/config.h>
#include <random>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace pfss;

//----------------------------------------------------------------------
// Logging
//----------------------------------------------------------------------

namespace {

struct pre_call_info_t {
  decltype(+pfss::unix_time_ns()) unix_time_ns = 0;
  decltype(+pfss::mono_time_ns()) mono_time_ns = 0;
  pre_call_info_t() {
    if (PFSS_WITH_LOGGING) {
      unix_time_ns = pfss::unix_time_ns();
      mono_time_ns = pfss::mono_time_ns();
    }
  }
};

struct post_call_info_t {
  decltype(+pfss::mono_time_ns()) mono_time_ns = 0;
  post_call_info_t() {
    if (PFSS_WITH_LOGGING) {
      mono_time_ns = pfss::mono_time_ns();
    }
  }
};

char const * pfss_get_status_name_nolog(pfss_status);

using boxed_status = pfss::boxed<pfss_status, struct boxed_status_tag>;

void print_arg(std::ostringstream & log_entry,
               boxed_status const & arg) {
  log_entry << pfss_get_status_name_nolog(arg.value());
}

template<
    class T,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
void print_arg(std::ostringstream & log_entry, T const & arg) {
  log_entry << pfss::to_string(arg);
}

template<class T>
void print_arg(std::ostringstream & log_entry, T const * const arg) {
  if (arg == nullptr) {
    log_entry << "NULL";
  } else {
    log_entry << static_cast<void const *>(arg);
  }
}

void print_arg(std::ostringstream & log_entry, FILE const * const arg) {
  if (arg == stdin) {
    log_entry << "stdin";
  } else if (arg == stdout) {
    log_entry << "stdout";
  } else if (arg == stderr) {
    log_entry << "stderr";
  } else {
    print_arg(log_entry, static_cast<void const *>(arg));
  }
}

void print_args(std::ostringstream &) noexcept {
}

template<class Arg, class... Args>
void print_args(std::ostringstream & log_entry,
                Arg const & head,
                Args const &... tail) {
  print_arg(log_entry, head);
  if (sizeof...(Args) != 0) {
    log_entry << ", ";
    print_args(log_entry, tail...);
  }
}

template<class Ret, class... Args>
void print_call(FILE * const log_file,
                pre_call_info_t const & pre_call_info,
                post_call_info_t const & post_call_info,
                char const * const func,
                Ret const & ret,
                Args const &... args) noexcept {
  try {
    assert(log_file != nullptr);
    assert(func != nullptr);
    std::ostringstream log_entry;
    print_arg(log_entry, pre_call_info.unix_time_ns);
    log_entry << " " << func << "(";
    print_args(log_entry, args...);
    log_entry << ") = ";
    print_arg(log_entry, ret);
    log_entry << " <";
    print_arg(log_entry,
              post_call_info.mono_time_ns - pre_call_info.mono_time_ns);
    log_entry << ">\n";
    static_cast<void>(fputs(log_entry.str().c_str(), log_file));
  } catch (...) {
  }
}

std::atomic<FILE *> g_log_file(nullptr);

FILE * pfss_set_log_file_nolog(FILE * const log_file) noexcept {
  return g_log_file.exchange(log_file);
}

FILE * pfss_get_log_file_nolog() noexcept {
  return g_log_file.load();
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
FILE * pfss_set_log_file(FILE * const log_file) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_set_log_file_nolog(log_file);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 ret,
                 log_file);
    }
  }
  return ret;
}

PFSS_PUBLIC_C_FUNCTION
FILE * pfss_get_log_file() {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_get_log_file_nolog();
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    if (ret != nullptr) {
      print_call(ret, pre_call_info, post_call_info, __func__, ret);
    }
  }
  return ret;
}

//----------------------------------------------------------------------

namespace {

constexpr uint32_t uint32_max = type_max<uint32_t>();

using uint_buf_type = uint_buf<uint32_t>;

//
//
//
//
//
//
//
//
//

template<class T>
void validate_domain_bits(T const domain_bits) {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
  if (domain_bits < 1) {
    throw error(PFSS_INVALID_DOMAIN);
  }
  if (domain_bits > 255) {
    throw error(PFSS_INVALID_DOMAIN);
  }
}

template<class T>
void validate_range_bits(T const range_bits) {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
  if (range_bits < 1) {
    throw error(PFSS_INVALID_RANGE);
  }
  if (range_bits > 255) {
    throw error(PFSS_INVALID_RANGE);
  }
}

template<class T>
void validate_pointer(T const * const p) {
  if (p == nullptr) {
    throw error(PFSS_NULL_POINTER);
  }
}

template<class T>
void validate_size(T const n) {
  if (n < 1) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
}

template<class T1, class T2>
void validate_byte_size(T1 const byte_size, T2 const minimum_bits) {
  if (byte_size < 1) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  if (unsigned_lt(byte_size, bits_to_bytes(minimum_bits))) {
    throw error(PFSS_RANGE_OVERFLOW);
  }
}

bool validate_endianness(pfss_endianness const e) {
  if (e == PFSS_LITTLE_ENDIAN) {
    return true;
  }
  if (e == PFSS_BIG_ENDIAN) {
    return false;
  }
  if (e != PFSS_NATIVE_ENDIAN) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  return system_is_little_endian();
}

template<class Scheme>
void inner_gen_sizes(
    Scheme const & scheme,
    uint32_t & key_blob_size,
    uint32_t & rand_buf_size) {
  {
    auto const n = scheme.key_blob_size();
    using T = decltype(n + uint32_max);
    if (static_cast<T>(n) > static_cast<T>(uint32_max)) {
      throw error(PFSS_UNKNOWN_ERROR);
    }
    key_blob_size = static_cast<uint32_t>(n);
  }
  {
    auto const n = scheme.rand_buf_size();
    using T = decltype(n + uint32_max);
    if (static_cast<T>(n) > static_cast<T>(uint32_max)) {
      throw error(PFSS_UNKNOWN_ERROR);
    }
    rand_buf_size = static_cast<uint32_t>(n);
  }
}

template<class Scheme>
void inner_gen(
    Scheme const & scheme,
    uint8_t const * const alpha,
    uint32_t const alpha_size,
    bool const alpha_is_little_endian,
    uint8_t const * const beta,
    uint32_t const beta_size,
    bool const beta_is_little_endian,
    uint8_t * const key1_blob,
    uint8_t * const key2_blob,
    uint8_t const * const rand_buf) {
  uint_buf_type const alpha_buf(
      alpha, alpha_size, alpha_is_little_endian);
  uint_buf_type const beta_buf(beta, beta_size, beta_is_little_endian);
  if (key1_blob == nullptr) {
    throw error(PFSS_NULL_POINTER);
  }
  if (key2_blob == nullptr) {
    throw error(PFSS_NULL_POINTER);
  }
  if (rand_buf == nullptr) {
    throw error(PFSS_NULL_POINTER);
  }
  typename Scheme::key_type keys[2] = {
      scheme.make_key(), scheme.make_key()};
  typename Scheme::rand_perm_type rand_perm;
  typename Scheme::rng_type rng(rand_buf, Scheme::rand_buf_size());
  scheme.gen(
      keys,
      alpha_buf,
      beta_buf.template get_as<typename Scheme::range_type>() &
          get_mask<typename Scheme::range_type>(scheme.range_bits),
      rand_perm,
      rng);
  keys[0].serialize(key1_blob);
  keys[1].serialize(key2_blob);
}

template<class Scheme>
void inner_eval(
    Scheme const & scheme,
    typename Scheme::key_type & key,
    typename Scheme::rand_perm_type & rand_perm,
    uint8_t const * const x,
    uint32_t const x_size,
    bool const x_is_little_endian,
    uint8_t * const y,
    uint32_t const y_size,
    bool const y_is_little_endian) {
  uint_buf_type const x_buf(x, x_size, x_is_little_endian);
  uint_buf_type y_buf(y, y_size, y_is_little_endian);
  y_buf = scheme.eval(key, x_buf, rand_perm);
}

} // namespace

//----------------------------------------------------------------------

namespace {

char const * pfss_get_status_name_nolog(pfss_status const status) {
  // clang-format off
  switch (status) {
#define C(x) case x: return #x
    C(PFSS_OK);
    C(PFSS_UNKNOWN_ERROR);
    C(PFSS_INVALID_ARGUMENT);
    C(PFSS_NULL_POINTER);
    C(PFSS_INVALID_DOMAIN);
    C(PFSS_INVALID_RANGE);
    C(PFSS_UNSUPPORTED_DOMAIN_AND_RANGE);
    C(PFSS_DOMAIN_OVERFLOW);
    C(PFSS_RANGE_OVERFLOW);
    C(PFSS_MALFORMED_KEY);
#undef C
  }
  // clang-format on
  return "PFSS_UNKNOWN_STATUS";
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
char const * pfss_get_status_name(pfss_status const status) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_get_status_name_nolog(status);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 ret,
                 boxed_status(status));
    }
  }
  return ret;
}

//----------------------------------------------------------------------
//
//

template<int RangeBits>
struct pick_range_type {};

template<>
struct pick_range_type<8> {
  using type = uint8_t;
};

template<>
struct pick_range_type<16> {
  using type = uint16_t;
};

template<>
struct pick_range_type<32> {
  using type = uint32_t;
};

template<>
struct pick_range_type<64> {
  using type = uint64_t;
};

//
//

using block_type = default_block_t<>;
using rand_perm_type = default_rand_perm_t<>;

template<
    template<int, int, class...>
    class Scheme,
    int DomainBits,
    int RangeBits>
using scheme_type = Scheme<
    DomainBits,
    RangeBits,
    uint_buf_type,
    typename pick_range_type<RangeBits>::type,
    block_type,
    rand_perm_type,
    buffered_rng<block_type>>;

template<template<class...> class Scheme>
using scheme_type_v = Scheme<
    uint_buf_type,
    unsigned long long,
    block_type,
    rand_perm_type,
    buffered_rng<block_type>>;

template<int DomainBits, int RangeBits>
using default_scheme_type =
    scheme_type<pfss::bgi1, DomainBits, RangeBits>;

using default_scheme_type_v = scheme_type_v<pfss::bgi1_v>;

//----------------------------------------------------------------------
// pfss_gen_sizes
//----------------------------------------------------------------------

namespace {

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = default_scheme_type<DomainBits, RangeBits>; \
    inner_gen_sizes(Scheme(), *key_blob_size, *rand_buf_size); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = default_scheme_type_v; \
    inner_gen_sizes( \
        Scheme( \
            static_cast<int>(domain_bits), \
            static_cast<int>(range_bits)), \
        *key_blob_size, \
        *rand_buf_size); \
  } while (0)

pfss_status pfss_gen_sizes_nolog(uint32_t const domain_bits,
                                 uint32_t const range_bits,
                                 uint32_t * const key_blob_size,
                                 uint32_t * const rand_buf_size) {
  try {
    validate_domain_bits(domain_bits);
    validate_range_bits(range_bits);
    validate_pointer(key_blob_size);
    validate_pointer(rand_buf_size);
    PFSS_SWITCH_DB_RB(domain_bits,
                      range_bits,
                      TEMPLATE_CODE,
                      STANDARD_CODE);
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_gen_sizes(uint32_t const domain_bits,
                           uint32_t const range_bits,
                           uint32_t * const key_blob_size,
                           uint32_t * const rand_buf_size) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_gen_sizes_nolog(domain_bits,
                                        range_bits,
                                        key_blob_size,
                                        rand_buf_size);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 domain_bits,
                 range_bits,
                 key_blob_size,
                 rand_buf_size);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_gen
//----------------------------------------------------------------------

namespace {

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = default_scheme_type<DomainBits, RangeBits>; \
    inner_gen( \
        Scheme(), \
        alpha, \
        alpha_size, \
        alpha_is_little_endian, \
        beta, \
        beta_size, \
        beta_is_little_endian, \
        key1_blob, \
        key2_blob, \
        rand_buf); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = default_scheme_type_v; \
    inner_gen( \
        Scheme( \
            static_cast<int>(domain_bits), \
            static_cast<int>(range_bits)), \
        alpha, \
        alpha_size, \
        alpha_is_little_endian, \
        beta, \
        beta_size, \
        beta_is_little_endian, \
        key1_blob, \
        key2_blob, \
        rand_buf); \
  } while (0)

pfss_status pfss_gen_nolog(uint32_t const domain_bits,
                           uint32_t const range_bits,
                           uint8_t const * const alpha,
                           uint32_t const alpha_size,
                           pfss_endianness const alpha_endianness,
                           uint8_t const * const beta,
                           uint32_t const beta_size,
                           pfss_endianness const beta_endianness,
                           uint8_t * const key1_blob,
                           uint8_t * const key2_blob,
                           uint8_t const * const rand_buf) {
  try {
    validate_domain_bits(domain_bits);
    validate_range_bits(range_bits);
    validate_pointer(alpha);
    validate_size(alpha_size);
    bool const alpha_is_little_endian =
        validate_endianness(alpha_endianness);
    validate_pointer(beta);
    validate_size(beta_size);
    bool const beta_is_little_endian =
        validate_endianness(beta_endianness);
    validate_pointer(key1_blob);
    validate_pointer(key2_blob);
    validate_pointer(rand_buf);
    PFSS_SWITCH_DB_RB(domain_bits,
                      range_bits,
                      TEMPLATE_CODE,
                      STANDARD_CODE);
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_gen(uint32_t const domain_bits,
                     uint32_t const range_bits,
                     uint8_t const * const alpha,
                     uint32_t const alpha_size,
                     pfss_endianness const alpha_endianness,
                     uint8_t const * const beta,
                     uint32_t const beta_size,
                     pfss_endianness const beta_endianness,
                     uint8_t * const key1_blob,
                     uint8_t * const key2_blob,
                     uint8_t const * const rand_buf) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_gen_nolog(domain_bits,
                                  range_bits,
                                  alpha,
                                  alpha_size,
                                  alpha_endianness,
                                  beta,
                                  beta_size,
                                  beta_endianness,
                                  key1_blob,
                                  key2_blob,
                                  rand_buf);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 domain_bits,
                 range_bits,
                 alpha,
                 alpha_size,
                 alpha_endianness,
                 beta,
                 beta_size,
                 beta_endianness,
                 key1_blob,
                 key2_blob,
                 rand_buf);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_map_gen
//----------------------------------------------------------------------

namespace {

#define COMMON_CODE() \
  do { \
    typename Scheme::key_type keys[2] = { \
        scheme.make_key(), scheme.make_key()}; \
    typename Scheme::rand_perm_type rand_perm; \
    while (alphas_count--) { \
      uint_buf_type const alpha_buf( \
          alphas, alpha_size, alpha_is_little_endian); \
      uint_buf_type const beta_buf( \
          betas, beta_size, beta_is_little_endian); \
      typename Scheme::rng_type rng( \
          rand_bufs, scheme.rand_buf_size()); \
      scheme.gen( \
          keys, \
          alpha_buf, \
          beta_buf.template get_as<typename Scheme::range_type>() & \
              get_mask<typename Scheme::range_type>( \
                  scheme.range_bits), \
          rand_perm, \
          rng); \
      keys[0].serialize(key1_blobs); \
      keys[1].serialize(key2_blobs); \
      alphas += alpha_size; \
      betas += beta_size; \
      key1_blobs += scheme.key_blob_size(); \
      key2_blobs += scheme.key_blob_size(); \
      rand_bufs += scheme.rand_buf_size(); \
    } \
  } while (0)

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = default_scheme_type<DomainBits, RangeBits>; \
    Scheme const scheme(domain_bits, range_bits); \
    COMMON_CODE(); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = default_scheme_type_v; \
    Scheme const scheme( \
        static_cast<int>(domain_bits), static_cast<int>(range_bits)); \
    COMMON_CODE(); \
  } while (0)

pfss_status pfss_map_gen_nolog(uint32_t const domain_bits,
                               uint32_t const range_bits,
                               uint8_t const * alphas,
                               uint32_t alphas_count,
                               uint32_t const alpha_size,
                               pfss_endianness const alpha_endianness,
                               uint8_t const * betas,
                               uint32_t const beta_size,
                               pfss_endianness const beta_endianness,
                               uint8_t * key1_blobs,
                               uint8_t * key2_blobs,
                               uint8_t const * rand_bufs) {
  try {
    validate_domain_bits(domain_bits);
    validate_range_bits(range_bits);
    validate_pointer(alphas);
    validate_size(alphas_count);
    validate_size(alpha_size);
    bool const alpha_is_little_endian =
        validate_endianness(alpha_endianness);
    validate_pointer(betas);
    validate_size(beta_size);
    bool const beta_is_little_endian =
        validate_endianness(beta_endianness);
    validate_pointer(key1_blobs);
    validate_pointer(key2_blobs);
    validate_pointer(rand_bufs);
    PFSS_SWITCH_DB_RB(domain_bits,
                      range_bits,
                      TEMPLATE_CODE,
                      STANDARD_CODE);
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

#undef COMMON_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_map_gen(uint32_t const domain_bits,
                         uint32_t const range_bits,
                         uint8_t const * alphas,
                         uint32_t alphas_count,
                         uint32_t const alpha_size,
                         pfss_endianness const alpha_endianness,
                         uint8_t const * betas,
                         uint32_t const beta_size,
                         pfss_endianness const beta_endianness,
                         uint8_t * key1_blobs,
                         uint8_t * key2_blobs,
                         uint8_t const * rand_bufs) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_map_gen_nolog(domain_bits,
                                      range_bits,
                                      alphas,
                                      alphas_count,
                                      alpha_size,
                                      alpha_endianness,
                                      betas,
                                      beta_size,
                                      beta_endianness,
                                      key1_blobs,
                                      key2_blobs,
                                      rand_bufs);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 domain_bits,
                 range_bits,
                 alphas,
                 alphas_count,
                 alpha_size,
                 alpha_endianness,
                 betas,
                 beta_size,
                 beta_endianness,
                 key1_blobs,
                 key2_blobs,
                 rand_bufs);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_parse_key
//----------------------------------------------------------------------

struct pfss_key {
  std::array<unsigned char, 1> header;
  int domain_bits;
  int range_bits;
  void * inner_key;
};

namespace {

#define COMMON_CODE() \
  do { \
    Scheme const scheme(key->domain_bits, key->range_bits); \
    auto inner_key = scheme.new_key(); \
    if (key_blob_size != scheme.key_blob_size()) { \
      throw error(PFSS_MALFORMED_KEY); \
    } \
    inner_key->parse(key_blob); \
    key->inner_key = inner_key.release(); \
  } while (0)

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = scheme_type<SCHEME_CLASS, DomainBits, RangeBits>; \
    COMMON_CODE(); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = scheme_type_v<SCHEME_CLASS_V>; \
    COMMON_CODE(); \
  } while (0)

pfss_status pfss_parse_key_nolog(
    pfss_key ** const p_key,
    uint8_t const * const key_blob,
    uint32_t const key_blob_size) {
  try {
    validate_pointer(p_key);
    validate_pointer(key_blob);
    validate_size(key_blob_size);
    std::unique_ptr<pfss_key> key(new pfss_key);
    if (key_blob[0] == 0) {
      if (key_blob_size < 3) {
        throw error(PFSS_MALFORMED_KEY);
      }
      std::copy_n(key_blob, 1, key->header.begin());
      key->domain_bits = static_cast<int>(key_blob[1]);
      key->range_bits = static_cast<int>(key_blob[2]);
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(
          key->domain_bits,
          key->range_bits,
          TEMPLATE_CODE,
          STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
    *p_key = key.release();
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

#undef COMMON_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_parse_key(pfss_key ** const p_key,
                           uint8_t const * const key_blob,
                           uint32_t const key_blob_size) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_parse_key_nolog(p_key, key_blob, key_blob_size);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 p_key,
                 key_blob,
                 key_blob_size);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_destroy_key
//----------------------------------------------------------------------

namespace {

#define COMMON_CODE() \
  do { \
    Scheme const scheme(key->domain_bits, key->range_bits); \
    scheme.delete_key(k->inner_key); \
  } while (0)

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = scheme_type<SCHEME_CLASS, DomainBits, RangeBits>; \
    COMMON_CODE(); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = scheme_type_v<SCHEME_CLASS_V>; \
    COMMON_CODE(); \
  } while (0)

pfss_status pfss_destroy_key_nolog(pfss_key * const key) {
  try {
    validate_pointer(key);
    std::unique_ptr<pfss_key> k(key);
    if (k->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(
          k->domain_bits, k->range_bits, TEMPLATE_CODE, STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

#undef COMMON_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_destroy_key(pfss_key * const key) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_destroy_key_nolog(key);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_get_domain_bits
//----------------------------------------------------------------------

namespace {

pfss_status pfss_get_domain_bits_nolog(pfss_key const * const key,
                                       uint32_t * const domain_bits) {
  try {
    validate_pointer(key);
    validate_pointer(domain_bits);
    if (key->header[0] == 0) {
      *domain_bits = static_cast<uint32_t>(key->domain_bits);
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_get_domain_bits(pfss_key const * const key,
                                 uint32_t * const domain_bits) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_get_domain_bits_nolog(key, domain_bits);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key,
                 domain_bits);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_get_range_bits
//----------------------------------------------------------------------

namespace {

pfss_status pfss_get_range_bits_nolog(pfss_key const * const key,
                                      uint32_t * const range_bits) {
  try {
    validate_pointer(key);
    validate_pointer(range_bits);
    if (key->header[0] == 0) {
      *range_bits = static_cast<uint32_t>(key->range_bits);
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_get_range_bits(pfss_key const * const key,
                                uint32_t * const range_bits) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_get_range_bits_nolog(key, range_bits);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key,
                 range_bits);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_eval
//----------------------------------------------------------------------

namespace {

#define COMMON_CODE() \
  do { \
    Scheme const scheme(key->domain_bits, key->range_bits); \
    Scheme::rand_perm_type rand_perm; \
    inner_eval<Scheme>( \
        scheme, \
        scheme.as_key(key->inner_key), \
        rand_perm, \
        x, \
        x_size, \
        x_is_little_endian, \
        y, \
        y_size, \
        y_is_little_endian); \
  } while (0)

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = scheme_type<SCHEME_CLASS, DomainBits, RangeBits>; \
    COMMON_CODE(); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = scheme_type_v<SCHEME_CLASS_V>; \
    COMMON_CODE(); \
  } while (0)

pfss_status pfss_eval_nolog(
    pfss_key const * const key,
    uint8_t const * const x,
    uint32_t const x_size,
    pfss_endianness const x_endianness,
    uint8_t * const y,
    uint32_t const y_size,
    pfss_endianness const y_endianness) {
  try {
    validate_pointer(key);
    validate_pointer(x);
    validate_size(x_size);
    bool const x_is_little_endian = validate_endianness(x_endianness);
    validate_pointer(y);
    validate_byte_size(y_size, key->range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    if (key->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(
          key->domain_bits,
          key->range_bits,
          TEMPLATE_CODE,
          STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

#undef COMMON_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval(pfss_key const * const key,
                      uint8_t const * const x,
                      uint32_t const x_size,
                      pfss_endianness const x_endianness,
                      uint8_t * const y,
                      uint32_t const y_size,
                      pfss_endianness const y_endianness) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_eval_nolog(key,
                                   x,
                                   x_size,
                                   x_endianness,
                                   y,
                                   y_size,
                                   y_endianness);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key,
                 x,
                 x_size,
                 x_endianness,
                 y,
                 y_size,
                 y_endianness);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_map_eval
//----------------------------------------------------------------------

namespace {

#define COMMON_CODE() \
  do { \
    Scheme const scheme(key->domain_bits, key->range_bits); \
    Scheme::rand_perm_type rand_perm; \
    while (xs_count--) { \
      uint_buf_type const x_buf(xs, x_size, x_is_little_endian); \
      uint_buf_type y_buf(ys, y_size, y_is_little_endian); \
      y_buf = scheme.eval( \
          scheme.as_key(key->inner_key), x_buf, rand_perm); \
      xs += x_size; \
      ys += y_size; \
    } \
  } while (0)

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = scheme_type<SCHEME_CLASS, DomainBits, RangeBits>; \
    COMMON_CODE(); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = scheme_type_v<SCHEME_CLASS_V>; \
    COMMON_CODE(); \
  } while (0)

pfss_status pfss_map_eval_nolog(
    pfss_key const * const key,
    uint8_t const * xs,
    uint32_t xs_count,
    uint32_t const x_size,
    pfss_endianness const x_endianness,
    uint8_t * ys,
    uint32_t const y_size,
    pfss_endianness const y_endianness) {
  try {
    validate_pointer(key);
    validate_pointer(xs);
    validate_size(xs_count);
    validate_size(x_size);
    bool const x_is_little_endian = validate_endianness(x_endianness);
    validate_pointer(ys);
    validate_byte_size(y_size, key->range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    if (key->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(
          key->domain_bits,
          key->range_bits,
          TEMPLATE_CODE,
          STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

#undef COMMON_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_map_eval(pfss_key const * const key,
                          uint8_t const * xs,
                          uint32_t xs_count,
                          uint32_t const x_size,
                          pfss_endianness const x_endianness,
                          uint8_t * ys,
                          uint32_t const y_size,
                          pfss_endianness const y_endianness) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_map_eval_nolog(key,
                                       xs,
                                       xs_count,
                                       x_size,
                                       x_endianness,
                                       ys,
                                       y_size,
                                       y_endianness);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key,
                 xs,
                 xs_count,
                 x_size,
                 x_endianness,
                 ys,
                 y_size,
                 y_endianness);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_reduce_sum
//----------------------------------------------------------------------

// TODO: We should be able to use uint_buf here somehow.

namespace {

template<class Y, bool Aligned, bool Reverse>
Y reduce_sum(
    uint32_t const range_bits,
    uint8_t const * ys,
    uint32_t ys_count) noexcept {
  PFSS_STATIC_ASSERT(
      (std::is_same<Y, std::uint8_t>::value ||
       std::is_same<Y, std::uint16_t>::value ||
       std::is_same<Y, std::uint32_t>::value ||
       std::is_same<Y, std::uint64_t>::value));
  assert(range_bits > 0);
  assert(bits_to_bytes(range_bits) <= sizeof(Y));
  assert(ys != nullptr);
  assert(ys_count > 0);
  typename promote_unsigned<Y>::type sum = 0;
  while (ys_count--) {
    Y y;
    if (Aligned) {
      auto const p = reinterpret_cast<Y const *>(ys);
      if (Reverse) {
        y = reverse_bytes(*p);
      } else {
        y = *p;
      }
    } else {
      auto const q = reinterpret_cast<uint8_t *>(&y);
      if (Reverse) {
        std::reverse_copy(ys, ys + sizeof(Y), q);
      } else {
        std::copy(ys, ys + sizeof(Y), q);
      }
    }
    sum += y;
    ys += sizeof(Y);
  }
  return static_cast<Y>(
      sum & get_mask<Y>(static_cast<int>(range_bits)));
}

pfss_status pfss_reduce_sum_nolog(
    uint32_t const range_bits,
    uint8_t const * const ys,
    uint32_t const ys_count,
    uint32_t const y_size,
    pfss_endianness const y_endianness,
    uint8_t * const z,
    uint32_t const z_size,
    pfss_endianness const z_endianness) {
  try {
    validate_range_bits(range_bits);
    validate_pointer(ys);
    validate_size(ys_count);
    validate_byte_size(y_size, range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    validate_pointer(z);
    validate_byte_size(z_size, range_bits);
    bool const z_is_little_endian = validate_endianness(z_endianness);
    uint_buf_type z_buf(z, z_size, z_is_little_endian);
    switch (y_size) {
      case 1: {
        z_buf = reduce_sum<std::uint8_t, true, false>(
            range_bits, ys, ys_count);
      } break;
#define C(Y) \
  case sizeof(Y): { \
    bool const aligned = is_aligned<Y>(ys); \
    bool const reverse = \
        y_is_little_endian != system_is_little_endian(); \
    if (aligned) { \
      if (reverse) { \
        z_buf = reduce_sum<Y, true, true>(range_bits, ys, ys_count); \
      } else { \
        z_buf = reduce_sum<Y, true, false>(range_bits, ys, ys_count); \
      } \
    } else { \
      if (reverse) { \
        z_buf = reduce_sum<Y, false, true>(range_bits, ys, ys_count); \
      } else { \
        z_buf = reduce_sum<Y, false, false>(range_bits, ys, ys_count); \
      } \
    } \
  } break
        C(std::uint16_t);
        C(std::uint32_t);
        C(std::uint64_t);
#undef C
      default: {
        throw error(PFSS_UNKNOWN_ERROR);
      } break;
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_reduce_sum(uint32_t const range_bits,
                            uint8_t const * const ys,
                            uint32_t const ys_count,
                            uint32_t const y_size,
                            pfss_endianness const y_endianness,
                            uint8_t * const z,
                            uint32_t const z_size,
                            pfss_endianness const z_endianness) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_reduce_sum_nolog(range_bits,
                                         ys,
                                         ys_count,
                                         y_size,
                                         y_endianness,
                                         z,
                                         z_size,
                                         z_endianness);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 range_bits,
                 ys,
                 ys_count,
                 y_size,
                 y_endianness,
                 z,
                 z_size,
                 z_endianness);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_map_eval_reduce_sum
//----------------------------------------------------------------------

namespace {

#define COMMON_CODE() \
  do { \
    Scheme const scheme(key->domain_bits, key->range_bits); \
    auto const & k = scheme.as_key(key->inner_key); \
    Scheme::rand_perm_type rand_perm; \
    typename promote_unsigned<typename Scheme::range_type>::type sum = \
        0; \
    uint_buf_type y_buf(y, y_size, y_is_little_endian); \
    while (xs_count--) { \
      uint_buf_type const x_buf(xs, x_size, x_is_little_endian); \
      sum += scheme.eval(k, x_buf, rand_perm); \
      xs += x_size; \
    } \
    y_buf = sum & get_mask<decltype(sum)>(scheme.range_bits); \
  } while (0)

#define TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = scheme_type<SCHEME_CLASS, DomainBits, RangeBits>; \
    COMMON_CODE(); \
  } while (0)

#define STANDARD_CODE() \
  do { \
    using Scheme = scheme_type_v<SCHEME_CLASS_V>; \
    COMMON_CODE(); \
  } while (0)

pfss_status pfss_map_eval_reduce_sum_nolog(
    pfss_key const * const key,
    uint8_t const * xs,
    uint32_t xs_count,
    uint32_t const x_size,
    pfss_endianness const x_endianness,
    uint8_t * const y,
    uint32_t const y_size,
    pfss_endianness const y_endianness) {
  try {
    validate_pointer(key);
    validate_pointer(xs);
    validate_size(xs_count);
    validate_size(x_size);
    bool const x_is_little_endian = validate_endianness(x_endianness);
    validate_pointer(y);
    validate_byte_size(y_size, key->range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    if (key->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(
          key->domain_bits,
          key->range_bits,
          TEMPLATE_CODE,
          STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

#undef STANDARD_CODE

#undef TEMPLATE_CODE

#undef COMMON_CODE

}

PFSS_PUBLIC_C_FUNCTION
pfss_status
pfss_map_eval_reduce_sum(pfss_key const * const key,
                         uint8_t const * xs,
                         uint32_t xs_count,
                         uint32_t const x_size,
                         pfss_endianness const x_endianness,
                         uint8_t * const y,
                         uint32_t const y_size,
                         pfss_endianness const y_endianness) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_map_eval_reduce_sum_nolog(key,
                                                  xs,
                                                  xs_count,
                                                  x_size,
                                                  x_endianness,
                                                  y,
                                                  y_size,
                                                  y_endianness);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key,
                 xs,
                 xs_count,
                 x_size,
                 x_endianness,
                 y,
                 y_size,
                 y_endianness);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_eval_all
//----------------------------------------------------------------------

namespace {

#define PFSS_EVAL_ALL_COMMON_CODE() \
  do { \
    Scheme const scheme(key->domain_bits, key->range_bits); \
    auto const & k = scheme.as_key(key->inner_key); \
    Scheme::rand_perm_type rand_perm; \
    auto cache = scheme.make_pack_eval_cache(); \
    int b = 0; \
    while (true) { \
      if (false && (x & to_unsigned(scheme.pack_mask)) == 0 \
          && x_last - x >= 3 \
          && unsigned_ge((x_last - x - 3) / 4, scheme.pack_mask)) { \
        /* \
        auto const x0 = x; \
        auto const x1 = x0 + to_unsigned(scheme.pack_count); \
        auto const x2 = x1 + to_unsigned(scheme.pack_count); \
        auto const x3 = x2 + to_unsigned(scheme.pack_count); \
        uint_buf_type const & x0_buf = x_buf; \
        uint_buf_type const x1_buf(&x1, \
                                   sizeof(x1), \
                                   system_is_little_endian()); \
        uint_buf_type const x2_buf(&x2, \
                                   sizeof(x2), \
                                   system_is_little_endian()); \
        uint_buf_type const x3_buf(&x3, \
                                   sizeof(x3), \
                                   system_is_little_endian()); \
        out = scheme.pack_eval_x4(k, \
                                  x0_buf, \
                                  x1_buf, \
                                  x2_buf, \
                                  x3_buf, \
                                  rand_perm, \
                                  out); \
        x = x3 | to_unsigned(scheme.pack_mask); \
        if (x == x_last) { \
          break; \
        } \
        ++x; \
        */ \
      } else if ((x & to_unsigned(scheme.pack_mask)) == 0 \
                 && unsigned_ge(x_last - x, scheme.pack_mask)) { \
        out = scheme.pack_eval(k, x_buf, rand_perm, cache, b, out); \
        auto const x_prev = x; \
        x |= pfss::to_unsigned(scheme.pack_mask); \
        if (x == x_last) { \
          break; \
        } \
        ++x; \
        b = scheme.v - 1 \
            - pfss::bit_scan_reverse((x_prev ^ x) \
                                     >> scheme.n_minus_v); \
      } else { \
        *out++ = scheme.eval(k, x_buf, rand_perm); \
        if (x == x_last) { \
          break; \
        } \
        ++x; \
        b = 0; \
      } \
    } \
  } while (0)

#define PFSS_EVAL_ALL_TEMPLATE_CODE(DomainBits, RangeBits) \
  do { \
    using Scheme = scheme_type<SCHEME_CLASS, DomainBits, RangeBits>; \
    PFSS_EVAL_ALL_COMMON_CODE(); \
  } while (0)

#define PFSS_EVAL_ALL_STANDARD_CODE() \
  do { \
    using Scheme = scheme_type_v<SCHEME_CLASS_V>; \
    PFSS_EVAL_ALL_COMMON_CODE(); \
  } while (0)

pfss_status pfss_eval_all_nolog(
    pfss_key const * const key,
    uint8_t const * const xp,
    uint32_t const xp_bits,
    pfss_endianness const xp_endianness,
    uint8_t * const ys,
    uint32_t const y_size,
    pfss_endianness const y_endianness) {
  try {
    validate_pointer(key);
    validate_pointer(xp);
    bool const xp_is_little_endian = validate_endianness(xp_endianness);
    validate_pointer(ys);
    validate_byte_size(y_size, key->range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    if (unsigned_gt(xp_bits, key->domain_bits)) {
      throw error(PFSS_DOMAIN_OVERFLOW);
    }
    if (unsigned_gt(key->domain_bits, type_bits<std::size_t>())) {
      throw error(PFSS_DOMAIN_OVERFLOW);
    }
    int const rbits = key->domain_bits - static_cast<int>(xp_bits);
    std::size_t x = 0;
    if (xp_bits != 0) {
      uint_buf_type const xp_buf(
          xp, bits_to_bytes(xp_bits), xp_is_little_endian);
      for (decltype(+xp_bits) i = 0; i != xp_bits; ++i) {
        x <<= 1;
        x |= static_cast<std::size_t>(xp_buf.getbitx(i));
      }
      x <<= rbits;
    }
    std::size_t const x_last = x | get_mask<std::size_t>(rbits);
    // TODO: x_buf should go away after we make the scheme classes more
    // flexible regarding the type of alpha and x, i.e., once DomainType
    // is moved from a template class parameter to a template function
    // parameter only on those functions. Because then we can use x
    // directly.
    uint_buf_type const x_buf(
        &x, sizeof(x), system_is_little_endian());
    uint_buf_iterator<uint32_t> out(ys + x * y_size, y_size, y_is_little_endian);
    if (key->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(
          key->domain_bits,
          key->range_bits,
          PFSS_EVAL_ALL_TEMPLATE_CODE,
          PFSS_EVAL_ALL_STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

}

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval_all(pfss_key const * const key,
                          uint8_t const * const xp,
                          uint32_t const xp_bits,
                          pfss_endianness const xp_endianness,
                          uint8_t * const ys,
                          uint32_t const y_size,
                          pfss_endianness const y_endianness) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_eval_all_nolog(key,
                                       xp,
                                       xp_bits,
                                       xp_endianness,
                                       ys,
                                       y_size,
                                       y_endianness);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 key,
                 xp,
                 xp_bits,
                 xp_endianness,
                 ys,
                 y_size,
                 y_endianness);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// Utilities for pfss_eval_all_{sum,dot}
//----------------------------------------------------------------------

namespace {

// Returns min(T1_max, thread_count, 2^domain_bits).
template<class T1, class T2, class T3>
T1 get_num_threads(T2 const thread_count,
                   T3 const domain_bits) noexcept {
  assert(thread_count > 0);
  assert(domain_bits > 0);
  constexpr T1 T1_max = sst::type_max<T1>::value;
  constexpr int T1_bits = sst::width_bits<T1>::value;
  T1 const n = sst::unsigned_lt(thread_count, T1_max) ?
                   T1(thread_count) :
                   T1_max;
  if (sst::unsigned_ge(domain_bits, T1_bits)) {
    return n;
  }
  T1 const domain_size = T1(T1(1) << domain_bits);
  return sst::min(n, domain_size);
}

// Thread i will own step + (i < slop) + 1 cells.
template<class T1, class T2>
std::pair<T1, T1> get_step_and_slop(T1 const num_threads,
                                    T2 const domain_bits) noexcept {
  assert(num_threads > 0);
  assert(domain_bits > 0);
  constexpr T1 T1_max = sst::type_max<T1>::value;
  constexpr int T1_bits = sst::width_bits<T1>::value;
  assert(sst::unsigned_le(domain_bits, T1_bits));
  if (num_threads == 1) {
    return std::make_pair(get_mask<T1>(domain_bits), T1(0));
  }
  if (sst::unsigned_eq(domain_bits, T1_bits)) {
    return std::make_pair(T1(T1_max / num_threads - T1(1)),
                          T1(T1_max % num_threads + T1(1)));
  }
  T1 const domain_size = T1(T1(1) << domain_bits);
  return std::make_pair(T1(domain_size / num_threads - T1(1)),
                        T1(domain_size % num_threads));
}

} // namespace

//----------------------------------------------------------------------
// pfss_eval_all_sum
//----------------------------------------------------------------------

namespace {

template<class Y, bool Aligned, bool Reverse>
void pfss_eval_all_sum_helper_2(pfss_key const * const * const keys,
                                uint32_t const keys_count,
                                std::size_t const x_first,
                                std::size_t const x_last,
                                uint8_t * const ys) {
  PFSS_STATIC_ASSERT((std::is_same<Y, std::uint8_t>::value
                      || std::is_same<Y, std::uint16_t>::value
                      || std::is_same<Y, std::uint32_t>::value
                      || std::is_same<Y, std::uint64_t>::value));
  assert(keys != nullptr);
  assert(keys_count > 0);
  assert(x_first <= x_last);
  assert(ys != nullptr);
  for (uint32_t i = 0; i != keys_count; ++i) {
    pfss_key const * const key = keys[i];
    std::size_t x = x_first;
    uint_buf_type const x_buf(&x, sizeof(x), system_is_little_endian());
    uint_buf_iterator<
        uint_buf_add_eq<uint_buf<void, Y, Aligned, Reverse>>>
        out(ys + x * sizeof(Y));
    if (key->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(key->domain_bits,
                        key->range_bits,
                        PFSS_EVAL_ALL_TEMPLATE_CODE,
                        PFSS_EVAL_ALL_STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  }
  {
    std::size_t x = x_first;
    uint_buf_type const x_buf(&x, sizeof(x), system_is_little_endian());
    uint_buf_iterator<
        uint_buf_and_eq<uint_buf<void, Y, Aligned, Reverse>>>
        out(ys + x * sizeof(Y));
    auto const mask = get_mask<Y>(keys[0]->range_bits);
    while (true) {
      *out++ = mask;
      if (x == x_last) {
        break;
      }
      ++x;
    }
  }
}

void pfss_eval_all_sum_helper_1(pfss_key const * const * const keys,
                                uint32_t const keys_count,
                                std::size_t const x_first,
                                std::size_t const x_last,
                                uint8_t * const ys,
                                uint32_t const y_size,
                                bool const y_is_little_endian) {
  switch (y_size) {
    case 1: {
      pfss_eval_all_sum_helper_2<std::uint8_t, true, false>(keys,
                                                            keys_count,
                                                            x_first,
                                                            x_last,
                                                            ys);
    } break;
#define C(Y)                                                           \
  case sizeof(Y): {                                                    \
    bool const aligned = is_aligned<Y>(ys);                            \
    bool const reverse =                                               \
        y_is_little_endian != system_is_little_endian();               \
    if (aligned) {                                                     \
      if (reverse) {                                                   \
        pfss_eval_all_sum_helper_2<Y, true, true>(keys,                \
                                                  keys_count,          \
                                                  x_first,             \
                                                  x_last,              \
                                                  ys);                 \
      } else {                                                         \
        pfss_eval_all_sum_helper_2<Y, true, false>(keys,               \
                                                   keys_count,         \
                                                   x_first,            \
                                                   x_last,             \
                                                   ys);                \
      }                                                                \
    } else {                                                           \
      if (reverse) {                                                   \
        pfss_eval_all_sum_helper_2<Y, false, true>(keys,               \
                                                   keys_count,         \
                                                   x_first,            \
                                                   x_last,             \
                                                   ys);                \
      } else {                                                         \
        pfss_eval_all_sum_helper_2<Y, false, false>(keys,              \
                                                    keys_count,        \
                                                    x_first,           \
                                                    x_last,            \
                                                    ys);               \
      }                                                                \
    }                                                                  \
  } break
      C(std::uint16_t);
      C(std::uint32_t);
      C(std::uint64_t);
#undef C
    default: {
      throw error(PFSS_UNKNOWN_ERROR);
    } break;
  }
}

void pfss_eval_all_sum_noval(pfss_key const * const * const keys,
                             uint32_t const keys_count,
                             uint8_t * const ys,
                             uint32_t const y_size,
                             bool const y_is_little_endian,
                             uint32_t const thread_count) {
  int const domain_bits = keys[0]->domain_bits;
  std::size_t const num_threads =
      get_num_threads<std::size_t>(thread_count, domain_bits);
  std::pair<std::size_t, std::size_t> const step_and_slop =
      get_step_and_slop(num_threads, domain_bits);
  std::size_t const step = step_and_slop.first;
  std::size_t const slop = step_and_slop.second;

  std::vector<std::future<void>> futures;
  futures.reserve(num_threads);

  std::size_t x_first = 0;
  for (std::size_t i = 0; i != num_threads; ++i) {
    std::size_t const x_last = x_first + (step + (i < slop));
    auto future = std::async(std::launch::async, [=] {
      pfss_eval_all_sum_helper_1(keys,
                                 keys_count,
                                 x_first,
                                 x_last,
                                 ys,
                                 y_size,
                                 y_is_little_endian);
    });
    try {
      futures.push_back(std::move(future));
    } catch (...) {
      try {
        future.get();
      } catch (...) {
      }
      for (std::size_t j = 0; j != i; ++j) {
        try {
          futures[j].get();
        } catch (...) {
        }
      }
      throw;
    }
    x_first = x_last + 1;
  }

  for (std::size_t i = 0; i != num_threads; ++i) {
    try {
      futures[i].get();
    } catch (...) {
      for (std::size_t j = i + 1; j != num_threads; ++j) {
        try {
          futures[j].get();
        } catch (...) {
        }
      }
      throw;
    }
  }
}

pfss_status
pfss_eval_all_sum_nolog(pfss_key const * const * const keys,
                        uint32_t const keys_count,
                        uint8_t * const ys,
                        uint32_t const y_size,
                        pfss_endianness const y_endianness,
                        uint32_t const thread_count) noexcept {
  try {
    validate_pointer(keys);
    validate_size(keys_count);
    for (uint32_t i = 0; i != keys_count; ++i) {
      validate_pointer(keys[i]);
      if (i > 0) {
        pfss_key const & a = *keys[i - 1];
        pfss_key const & b = *keys[i];
        if (b.domain_bits != a.domain_bits) {
          throw error(PFSS_INVALID_ARGUMENT);
        }
        if (b.range_bits != a.range_bits) {
          throw error(PFSS_INVALID_ARGUMENT);
        }
      }
    }
    if (sst::unsigned_gt(keys[0]->domain_bits,
                         sst::width_bits<std::size_t>::value)) {
      throw error(PFSS_DOMAIN_OVERFLOW);
    }
    validate_pointer(ys);
    validate_byte_size(y_size, keys[0]->range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    validate_size(thread_count);
    pfss_eval_all_sum_noval(keys,
                            keys_count,
                            ys,
                            y_size,
                            y_is_little_endian,
                            thread_count);
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval_all_sum(pfss_key const * const * const keys,
                              uint32_t const keys_count,
                              uint8_t * const ys,
                              uint32_t const y_size,
                              pfss_endianness const y_endianness,
                              uint32_t const thread_count) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_eval_all_sum_nolog(keys,
                                           keys_count,
                                           ys,
                                           y_size,
                                           y_endianness,
                                           thread_count);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 keys,
                 keys_count,
                 ys,
                 y_size,
                 y_endianness,
                 thread_count);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
// pfss_eval_all_dot
//----------------------------------------------------------------------

namespace {

template<class Y, bool Aligned, bool Reverse>
void pfss_eval_all_dot_helper_2(
    pfss_key const * const * const keys,
    uint32_t const keys_count,
    std::size_t const x_first,
    std::size_t const x_last,
    uint8_t const * const ys,
    std::vector<unsigned long long> & partials) {
  PFSS_STATIC_ASSERT((std::is_same<Y, std::uint8_t>::value
                      || std::is_same<Y, std::uint16_t>::value
                      || std::is_same<Y, std::uint32_t>::value
                      || std::is_same<Y, std::uint64_t>::value));
  assert(keys != nullptr);
  assert(keys_count > 0);
  assert(x_first <= x_last);
  assert(ys != nullptr);
  for (uint32_t i = 0; i != keys_count; ++i) {
    pfss_key const * const key = keys[i];
    std::size_t x = x_first;
    uint_buf_type const x_buf(&x, sizeof(x), system_is_little_endian());
    uint_buf_iterator<
        dot_accumulator<unsigned long long,
                        uint_buf<void, Y const, Aligned, Reverse>>>
        out(&partials[i], ys + x * sizeof(Y));
    if (key->header[0] == 0) {
#define SCHEME_CLASS pfss::bgi1
#define SCHEME_CLASS_V pfss::bgi1_v
      PFSS_SWITCH_DB_RB(key->domain_bits,
                        key->range_bits,
                        PFSS_EVAL_ALL_TEMPLATE_CODE,
                        PFSS_EVAL_ALL_STANDARD_CODE);
#undef SCHEME_CLASS_V
#undef SCHEME_CLASS
    } else {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  }
}

void pfss_eval_all_dot_helper_1(
    pfss_key const * const * const keys,
    uint32_t const keys_count,
    std::size_t const x_first,
    std::size_t const x_last,
    uint8_t const * const ys,
    uint32_t const y_size,
    bool const y_is_little_endian,
    std::vector<unsigned long long> & partials) {
  switch (y_size) {
    case 1: {
      pfss_eval_all_dot_helper_2<std::uint8_t, true, false>(keys,
                                                            keys_count,
                                                            x_first,
                                                            x_last,
                                                            ys,
                                                            partials);
    } break;
#define C(Y)                                                           \
  case sizeof(Y): {                                                    \
    bool const aligned = is_aligned<Y>(ys);                            \
    bool const reverse =                                               \
        y_is_little_endian != system_is_little_endian();               \
    if (aligned) {                                                     \
      if (reverse) {                                                   \
        pfss_eval_all_dot_helper_2<Y, true, true>(keys,                \
                                                  keys_count,          \
                                                  x_first,             \
                                                  x_last,              \
                                                  ys,                  \
                                                  partials);           \
      } else {                                                         \
        pfss_eval_all_dot_helper_2<Y, true, false>(keys,               \
                                                   keys_count,         \
                                                   x_first,            \
                                                   x_last,             \
                                                   ys,                 \
                                                   partials);          \
      }                                                                \
    } else {                                                           \
      if (reverse) {                                                   \
        pfss_eval_all_dot_helper_2<Y, false, true>(keys,               \
                                                   keys_count,         \
                                                   x_first,            \
                                                   x_last,             \
                                                   ys,                 \
                                                   partials);          \
      } else {                                                         \
        pfss_eval_all_dot_helper_2<Y, false, false>(keys,              \
                                                    keys_count,        \
                                                    x_first,           \
                                                    x_last,            \
                                                    ys,                \
                                                    partials);         \
      }                                                                \
    }                                                                  \
  } break
      C(std::uint16_t);
      C(std::uint32_t);
      C(std::uint64_t);
#undef C
    default: {
      throw error(PFSS_UNKNOWN_ERROR);
    } break;
  }
}

void pfss_eval_all_dot_noval(pfss_key const * const * const keys,
                             uint32_t const keys_count,
                             uint8_t const * const ys,
                             uint32_t const y_size,
                             bool const y_is_little_endian,
                             uint8_t * const zs,
                             uint32_t const z_size,
                             bool const z_is_little_endian,
                             uint32_t const thread_count) {
  int const domain_bits = keys[0]->domain_bits;
  int const range_bits = keys[0]->range_bits;

  std::size_t const num_threads =
      get_num_threads<std::size_t>(thread_count, domain_bits);
  std::pair<std::size_t, std::size_t> const step_and_slop =
      get_step_and_slop(num_threads, domain_bits);
  std::size_t const step = step_and_slop.first;
  std::size_t const slop = step_and_slop.second;

  std::vector<std::future<void>> futures;
  futures.reserve(num_threads);

  std::vector<std::vector<unsigned long long>> partials(
      num_threads,
      std::vector<unsigned long long>(keys_count));

  std::size_t x_first = 0;
  for (std::size_t i = 0; i != num_threads; ++i) {
    std::size_t const x_last = x_first + (step + (i < slop));
    auto future = std::async(std::launch::async, [=, &partials] {
      pfss_eval_all_dot_helper_1(keys,
                                 keys_count,
                                 x_first,
                                 x_last,
                                 ys,
                                 y_size,
                                 y_is_little_endian,
                                 partials[i]);
    });
    try {
      futures.push_back(std::move(future));
    } catch (...) {
      try {
        future.get();
      } catch (...) {
      }
      for (std::size_t j = 0; j != i; ++j) {
        try {
          futures[j].get();
        } catch (...) {
        }
      }
      throw;
    }
    x_first = x_last + 1;
  }

  for (std::size_t i = 0; i != num_threads; ++i) {
    try {
      futures[i].get();
    } catch (...) {
      for (std::size_t j = i + 1; j != num_threads; ++j) {
        try {
          futures[j].get();
        } catch (...) {
        }
      }
      throw;
    }
  }

  for (decltype(+num_threads) i = 1; i < num_threads; ++i) {
    for (decltype(+keys_count) j = 0; j < keys_count; ++j) {
      partials[0][j] += partials[i][j];
    }
  }
  unsigned char * z = zs;
  for (decltype(+keys_count) j = 0; j < keys_count; ++j) {
    uint_buf_type z_buf(z, z_size, z_is_little_endian);
    z_buf = partials[0][j] & get_mask<unsigned long long>(range_bits);
    z += z_size;
  }
}

pfss_status
pfss_eval_all_dot_nolog(pfss_key const * const * const keys,
                        uint32_t const keys_count,
                        uint8_t const * const ys,
                        uint32_t const y_size,
                        pfss_endianness const y_endianness,
                        uint8_t * const zs,
                        uint32_t const z_size,
                        pfss_endianness const z_endianness,
                        uint32_t const thread_count) noexcept {
  try {
    validate_pointer(keys);
    validate_size(keys_count);
    for (uint32_t i = 0; i != keys_count; ++i) {
      validate_pointer(keys[i]);
      if (i > 0) {
        pfss_key const & a = *keys[i - 1];
        pfss_key const & b = *keys[i];
        if (b.domain_bits != a.domain_bits) {
          throw error(PFSS_INVALID_ARGUMENT);
        }
        if (b.range_bits != a.range_bits) {
          throw error(PFSS_INVALID_ARGUMENT);
        }
      }
    }
    if (sst::unsigned_gt(keys[0]->domain_bits,
                         sst::width_bits<std::size_t>::value)) {
      throw error(PFSS_DOMAIN_OVERFLOW);
    }
    validate_pointer(ys);
    validate_byte_size(y_size, keys[0]->range_bits);
    bool const y_is_little_endian = validate_endianness(y_endianness);
    validate_pointer(zs);
    validate_byte_size(z_size, keys[0]->range_bits);
    bool const z_is_little_endian = validate_endianness(z_endianness);
    validate_size(thread_count);
    pfss_eval_all_dot_noval(keys,
                            keys_count,
                            ys,
                            y_size,
                            y_is_little_endian,
                            zs,
                            z_size,
                            z_is_little_endian,
                            thread_count);
  } catch (error const & e) {
    return e.status;
  } catch (...) {
    return PFSS_UNKNOWN_ERROR;
  }
  return PFSS_OK;
}

} // namespace

PFSS_PUBLIC_C_FUNCTION
pfss_status pfss_eval_all_dot(pfss_key const * const * const keys,
                              uint32_t const keys_count,
                              uint8_t const * const ys,
                              uint32_t const y_size,
                              pfss_endianness const y_endianness,
                              uint8_t * const zs,
                              uint32_t const z_size,
                              pfss_endianness const z_endianness,
                              uint32_t const thread_count) {
  pre_call_info_t pre_call_info;
  auto const ret = pfss_eval_all_dot_nolog(keys,
                                           keys_count,
                                           ys,
                                           y_size,
                                           y_endianness,
                                           zs,
                                           z_size,
                                           z_endianness,
                                           thread_count);
  post_call_info_t post_call_info;
  if (PFSS_WITH_LOGGING) {
    FILE * const log_file = g_log_file.load();
    if (log_file != nullptr) {
      print_call(log_file,
                 pre_call_info,
                 post_call_info,
                 __func__,
                 boxed_status(ret),
                 keys,
                 keys_count,
                 ys,
                 y_size,
                 y_endianness,
                 zs,
                 z_size,
                 z_endianness,
                 thread_count);
    }
  }
  return ret;
}

//----------------------------------------------------------------------
