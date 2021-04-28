//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_COMMON_H
#define PFSS_COMMON_H

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <pfss.h>
#include <pfss/base.hpp>
#include <pfss/config.h>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if PFSS_CPP20_OR_LATER
#include <bit>
#endif

#if PFSS_HAVE_AES_NI
#include <wmmintrin.h>
#endif

#if PFSS_HAVE_ARM_CRYPTO
#include <arm_neon.h>
#endif

#if PFSS_HAVE_AVX_512_F
#include <immintrin.h>
#endif

#if PFSS_HAVE_NETTLE
#include <nettle/aes.h>
#endif

#if PFSS_HAVE_SSE2
#include <emmintrin.h>
#endif

namespace pfss {

struct error final {
  pfss_status status;

  explicit constexpr error(pfss_status const status) : status(status) {
    PFSS_CONSTEXPR_ASSERT(status != PFSS_OK);
  }
};

template<class T>
constexpr int type_bits() noexcept {
  return std::numeric_limits<T>::digits;
}

constexpr int uchar_bits = type_bits<unsigned char>();
constexpr unsigned char uchar_max = type_max<unsigned char>();
constexpr unsigned char uchar_msb = uchar_max ^ (uchar_max >> 1);
constexpr unsigned char uchar_msb_neg = uchar_max >> 1;

constexpr std::size_t size_max = type_max<std::size_t>();

//
// Determines how many bytes are needed to hold n bits.
//

template<class T>
constexpr T bits_to_bytes(T const n) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
  PFSS_CONSTEXPR_ASSERT(n >= 0);
  return static_cast<T>(n / uchar_bits + (n % uchar_bits != 0));
}

//
// Determines whether a pointer to at least sizeof(T) bytes is suitably
// aligned for access via T *.
//

template<class T, class P>
bool is_aligned(P const * const p) noexcept {
  assert(p != nullptr);
  void * q = const_cast<P *>(p);
  std::size_t n = sizeof(T);
  return std::align(alignof(T), n, q, n) != nullptr;
}

//
// Determine whether the system is little or big endian.
//

#if PFSS_CPP20_OR_LATER
constexpr bool system_is_little_endian() noexcept {
  return std::endian::native == std::endian::little;
}
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
constexpr bool system_is_little_endian() noexcept {
  return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}
#else
inline bool system_is_little_endian() noexcept {
  std::uint64_t const x = 1;
  return *reinterpret_cast<unsigned char const *>(&x) == 1;
}
#endif

//
// Reverse the bytes of the underlying representation of an object.
//

template<class T>
T reverse_bytes(T x) {
  PFSS_STATIC_ASSERT(std::is_trivial<T>::value);
  unsigned char * const p = reinterpret_cast<unsigned char *>(&x);
  std::size_t const n = sizeof(x);
  for (std::size_t i = 0; i != n / 2; ++i) {
    std::swap(p[i], p[n - 1 - i]);
  }
  return x;
}

//
// Determine whether T is a non-cv-qualified integer type.
//

template<class, class = void>
struct is_non_cv_integer : std::false_type {};

template<class T>
struct is_non_cv_integer<
    T,
    typename std::enable_if<
        std::is_integral<T>::value && !std::is_const<T>::value &&
        !std::is_volatile<T>::value>::type> : std::true_type {};

//
// Determine whether T is a non-cv-qualified non-bool integer type.
//

template<class, class = void>
struct is_non_cvb_integer : std::false_type {};

template<class T>
struct is_non_cvb_integer<
    T,
    typename std::enable_if<
        is_non_cv_integer<T>::value &&
        !std::is_same<T, bool>::value>::type> : std::true_type {};

//
// Return 2^n - 1 for n > 0.
//

template<class T>
constexpr T get_mask(int const n) noexcept {
  PFSS_STATIC_ASSERT(is_non_cv_integer<T>::value);
  PFSS_CONSTEXPR_ASSERT(!pfss::is_negative(n));
  PFSS_CONSTEXPR_ASSERT(n <= type_bits<T>());
  return n == 0 ? static_cast<T>(0) :
                  static_cast<T>(type_max<T>() >> (type_bits<T>() - n));
}

//
// Returns the largest k such that 2^k <= x, or -1 if x <= 0 or x is too
// big to compute k properly. You can think of k as being one less than
// the number of bits in x.
//

// clang-format off
constexpr int power_of_two_width(int const x) {
  #define S(n) ((x >> n) == 0 && (x >> (n - 1)) != 0) ? n - 1 :
  return x <= 0 ? -1 :
    S( 1) S( 2) S( 3) S( 4) S( 5) S( 6) S( 7) S( 8)
    S( 9) S(10) S(11) S(12) S(13) S(14) S(15) S(16)
    S(17) S(18) S(19) S(20) S(21) S(22) S(23) S(24)
    S(25) S(26) S(27) S(28) S(29) S(30) S(31)
    -1
  ;
  #undef S
}
// clang-format on

// Define getbitx(x, i) for any integer x and any object x such that
// x.getbitx(i) is valid.
template<
    class T,
    class Index,
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
constexpr bool getbitx(T const x, Index const i) {
  return (x >> i) & 1;
}
template<
    class T,
    class Index,
    typename std::enable_if<
        std::is_same<
            decltype(std::declval<T>().getbitx(std::declval<Index>())),
            bool>::value,
        int>::type = 0>
constexpr bool getbitx(T const & x, Index const i) {
  return x.getbitx(i);
}

template<class T, class Dst>
Dst serialize_integer(int const bits, T src, Dst dst) noexcept {
  PFSS_STATIC_ASSERT(is_non_cvb_integer<T>::value);
  assert(bits >= 1);
  assert(src >= 0);
  int const n = bits / uchar_bits + (bits % uchar_bits != 0);
  for (int i = 0; i != n; ++i) {
    *dst++ = static_cast<unsigned char>(src);
    src = static_cast<T>(src >> 4);
    src = static_cast<T>(src >> 4);
  }
  return dst;
}

template<class T, class Src>
Src deserialize_integer(int const bits, T & dst, Src src) {
  PFSS_STATIC_ASSERT(is_non_cvb_integer<T>::value);
  assert(bits >= 1);
  assert(bits <= type_bits<T>());
  int const nq = bits / uchar_bits;
  int const nr = bits % uchar_bits;
  int const n = nq + (nr != 0);
  dst = 0;
  for (int i = 0; i != n; ++i) {
    unsigned char const b = *src++;
    if (nr != 0 && i == n - 1) {
      if (b > get_mask<unsigned char>(nr)) {
        throw error(PFSS_MALFORMED_KEY);
      }
    }
    dst = static_cast<T>(dst | (static_cast<T>(b) << (uchar_bits * i)));
  }
  return src;
}

constexpr bool endianness_is_valid(pfss_endianness const e) noexcept {
  return e == PFSS_NATIVE_ENDIAN || e == PFSS_LITTLE_ENDIAN ||
      e == PFSS_BIG_ENDIAN;
}

constexpr bool fold_endianness(pfss_endianness const e) noexcept {
  PFSS_CONSTEXPR_ASSERT(endianness_is_valid(e));
  return e == PFSS_NATIVE_ENDIAN ? system_is_little_endian() :
                                   e == PFSS_LITTLE_ENDIAN;
}

//----------------------------------------------------------------------
// uint_buf
//----------------------------------------------------------------------
//
// The uint_buf class wraps a region of memory and allows it to be
// treated as an unsigned integer with a particular endianness.
//

template<class SizeType = std::size_t,
         class = void,
         bool = true,
         bool = false>
class uint_buf final {

  template<class, class, bool, bool>
  friend class uint_buf;

  unsigned char * buf;
  SizeType size;
  bool little_endian;

  //
  // The portion of buf that stores the value, excluding any leading
  // (big endian) or trailing (little endian) zero bytes. If the value
  // is zero, then the portion is taken to be the singular byte at the
  // beginning (little endian) or end (big endian) of buf.
  //

  mutable SizeType vsize_value = 0;

  SizeType vsize() const noexcept {
    if (vsize_value == 0) {
      vsize_value = size;
      if (little_endian) {
        while (vsize_value != 1 && buf[vsize_value - 1] == 0) {
          --vsize_value;
        }
      } else {
        while (vsize_value != 1 && buf[size - vsize_value] == 0) {
          --vsize_value;
        }
      }
    }
    return vsize_value;
  }

  unsigned char * vbuf() noexcept {
    return little_endian ? buf : buf + zsize();
  }

  unsigned char const * vbuf() const noexcept {
    return little_endian ? buf : buf + zsize();
  }

  //
  // The portion of buf that stores the leading (big endian) or trailing
  // (little endian) zero bytes. This may be empty.
  //

  SizeType zsize() const noexcept {
    return size - vsize();
  }

  unsigned char * zbuf() noexcept {
    return little_endian ? buf + vsize() : buf;
  }

  unsigned char const * zbuf() const noexcept {
    return little_endian ? buf + vsize() : buf;
  }

  //
  // The copy_value_from function copies the value from another uint_buf
  // into this one. If the value is too big for our buffer, the behavior
  // is undefined.
  //

  template<class OtherSizeType>
  uint_buf &
  copy_value_from(uint_buf<OtherSizeType> const & other) noexcept {
    assert(size >= other.vsize());
    vsize_value = static_cast<SizeType>(other.vsize());
    if (little_endian == other.little_endian) {
      std::copy_n(other.vbuf(), vsize(), vbuf());
    } else {
      std::reverse_copy(other.vbuf(), other.vbuf() + vsize(), vbuf());
    }
    std::fill_n(zbuf(), zsize(), 0);
    return *this;
  }

public:
  explicit uint_buf(void const * const buf,
                    SizeType const size,
                    bool const little_endian)
      : buf(
          (assert(buf != nullptr),
           assert(size > 0),
           reinterpret_cast<unsigned char *>(const_cast<void *>(buf)))),
        size(size),
        little_endian(little_endian) {
  }

  ~uint_buf() noexcept = default;

  uint_buf(uint_buf const &) noexcept = default;

  uint_buf(uint_buf &&) noexcept = default;

  uint_buf & operator=(uint_buf const & other) noexcept {
    return copy_value_from(other);
  }

  uint_buf & operator=(uint_buf &&) = delete;

  template<class OtherSizeType>
  uint_buf & operator=(uint_buf<OtherSizeType> const & other) noexcept {
    return copy_value_from(other);
  }

  template<class IntType>
  uint_buf & operator=(IntType x) noexcept {
    PFSS_STATIC_ASSERT(std::is_integral<IntType>::value);
    assert(x >= 0);
    vsize_value = 0;
    for (SizeType i = 0; i != size; ++i) {
      unsigned char const b = static_cast<unsigned char>(x);
      buf[little_endian ? i : size - 1 - i] = b;
      if (b != 0) {
        vsize_value = i + 1;
      }
      x >>= uchar_bits - 1;
      x >>= 1;
    }
    assert(x == 0);
    return *this;
  }

  template<class T>
  bool operator==(uint_buf<T> const & other) const noexcept {
    if (vsize() != other.vsize()) {
      return false;
    }
    if (little_endian == other.little_endian) {
      return std::equal(vbuf(), vbuf() + vsize(), other.vbuf());
    }
    for (SizeType i = 0; i != vsize(); ++i) {
      if (vbuf()[i] != other.vbuf()[vsize() - 1 - i]) {
        return false;
      }
    }
    return true;
  }

  //
  // Returns the value mod type_max<T>() + 1.
  //

  template<class T>
  T get_as() const noexcept {
    PFSS_STATIC_ASSERT(std::is_integral<T>::value);
    typename promote_unsigned<T>::type x = 0;
    for (SizeType i = 0; i != vsize(); ++i) {
      x <<= uchar_bits - 1;
      x <<= 1;
      x |= vbuf()[little_endian ? vsize() - 1 - i : i];
    }
    return static_cast<T>(x & type_max<T>());
  }

  //
  // Returns the i'th bit of the value, where the LSB is the 0'th bit.
  // The value is treated as if it had infinitely many bits, i.e., the
  // function returns zero if i >= size * uchar_bits.
  //

  template<class T>
  bool getbitx(T const i) const noexcept {
    PFSS_STATIC_ASSERT(std::is_integral<T>::value);
    assert(i >= 0);
    using U = typename promote_unsigned<T>::type;
    U const q = static_cast<U>(i / uchar_bits);
    U const r = static_cast<U>(i % uchar_bits);
    if (q < size) {
      return (buf[little_endian ? q : size - 1 - q] >> r) & 1;
    }
    return 0;
  }

  //
  // Determines whether the value has at most n bits.
  //

  template<class T>
  bool fits(T const n) const noexcept {
    assert(n > 0);
    using U = typename promote_unsigned<T>::type;
    U const q = static_cast<U>(n / uchar_bits);
    U const r = static_cast<U>(n % uchar_bits);
    if (vsize() <= q) {
      return true;
    }
    if (vsize() > q + (r != 0)) {
      return false;
    }
    return (vbuf()[little_endian ? q : 0] >> r) == 0;
  }

  //
  // Sets the value to zero.
  //

  void zero() noexcept {
    std::fill_n(buf, size, 0);
    vsize_value = 1;
  }

  std::string to_string() const {
    std::string s;
    s += "{";
    for (SizeType i = 0; i != size; ++i) {
      if (i != 0) {
        s += ",";
      }
      s += std::to_string(buf[i]);
    }
    s += "}";
    return s;
  }
};

template<class ValueType, bool Aligned, bool Reversed>
class uint_buf<void, ValueType, Aligned, Reversed> final {
  PFSS_STATIC_ASSERT(
      (std::is_same<typename std::remove_const<ValueType>::type,
                    std::uint8_t>::value
       || std::is_same<typename std::remove_const<ValueType>::type,
                       std::uint16_t>::value
       || std::is_same<typename std::remove_const<ValueType>::type,
                       std::uint32_t>::value
       || std::is_same<typename std::remove_const<ValueType>::type,
                       std::uint64_t>::value));

  typename std::conditional<std::is_const<ValueType>::value,
                            unsigned char const,
                            unsigned char>::type * p_;

  void increment() noexcept {
    p_ += sizeof(ValueType);
  }

  template<class>
  friend class uint_buf_add_eq;

  template<class, class>
  friend class dot_accumulator;

  template<class>
  friend class uint_buf_and_eq;

  template<class, class>
  friend class uint_buf_iterator;

public:
  using value_type = typename std::remove_const<ValueType>::type;

  explicit uint_buf(void * const p) noexcept
      : p_((assert(p != nullptr), reinterpret_cast<decltype(p_)>(p))) {
  }

  template<
      class V = ValueType,
      typename std::enable_if<std::is_const<V>::value, int>::type = 0>
  explicit uint_buf(void const * const p) noexcept
      : p_((assert(p != nullptr), reinterpret_cast<decltype(p_)>(p))) {
  }

  ~uint_buf() noexcept = default;
  uint_buf(uint_buf const &) noexcept = default;
  uint_buf & operator=(uint_buf const &) noexcept = default;
  uint_buf(uint_buf &&) noexcept = default;
  uint_buf & operator=(uint_buf &&) noexcept = default;

  template<class T,
           typename std::enable_if<sst::is_unsigned_integer<T>::value,
                                   int>::type = 0>
  explicit operator T() const noexcept {
    value_type x;
    if (Aligned) {
      auto const p = reinterpret_cast<value_type const *>(p_);
      if (Reversed) {
        x = reverse_bytes(*p);
      } else {
        x = *p;
      }
    } else {
      auto const q = reinterpret_cast<unsigned char *>(&x);
      if (Reversed) {
        std::reverse_copy(p_, p_ + sizeof(value_type), q);
      } else {
        std::copy(p_, p_ + sizeof(value_type), q);
      }
    }
    return static_cast<T>(x);
  }

  template<class T,
           class V = ValueType,
           typename std::enable_if<!std::is_const<V>::value
                                       && std::is_integral<T>::value,
                                   int>::type = 0>
  uint_buf & operator=(T const b) noexcept {
    value_type const x = static_cast<value_type>(b);
    if (Aligned) {
      auto const p = reinterpret_cast<value_type *>(p_);
      if (Reversed) {
        *p = reverse_bytes(x);
      } else {
        *p = x;
      }
    } else {
      auto const q = reinterpret_cast<unsigned char const *>(&x);
      if (Reversed) {
        std::reverse_copy(q, q + sizeof(value_type), p_);
      } else {
        std::copy(q, q + sizeof(value_type), p_);
      }
    }
    return *this;
  }

#define PFSS_OP(op)                                                    \
                                                                       \
  template<class T,                                                    \
           typename std::enable_if<std::is_integral<T>::value,         \
                                   int>::type = 0>                     \
  auto operator op(T const b) const noexcept->decltype(                \
      sst::to_unsigned(value_type()) op sst::to_unsigned(T())) {       \
    return sst::to_unsigned(static_cast<value_type>(*this))            \
        op sst::to_unsigned(b);                                        \
  }                                                                    \
                                                                       \
  template<class T,                                                    \
           typename std::enable_if<std::is_integral<T>::value,         \
                                   int>::type = 0>                     \
  friend auto operator op(T const a, uint_buf const & b) noexcept      \
      ->decltype(sst::to_unsigned(T())                                 \
                     op sst::to_unsigned(uint_buf::value_type())) {    \
    return sst::to_unsigned(a)                                         \
        op sst::to_unsigned(static_cast<uint_buf::value_type>(b));     \
  }

  PFSS_OP(+)
  PFSS_OP(*)
  PFSS_OP(&)

#undef PFSS_OP

  template<class T,
           class V = ValueType,
           typename std::enable_if<std::is_integral<T>::value
                                       && !std::is_const<V>::value,
                                   int>::type = 0>
  uint_buf & operator+=(T const b) noexcept {
    return *this = sst::to_unsigned(static_cast<value_type>(*this))
                   + sst::to_unsigned(b);
  }

  template<class T,
           class V = ValueType,
           typename std::enable_if<std::is_integral<T>::value
                                       && !std::is_const<V>::value,
                                   int>::type = 0>
  uint_buf & operator*=(T const b) noexcept {
    return *this = sst::to_unsigned(static_cast<value_type>(*this))
                   * sst::to_unsigned(b);
  }

  template<class T,
           class V = ValueType,
           typename std::enable_if<std::is_integral<T>::value
                                       && !std::is_const<V>::value,
                                   int>::type = 0>
  uint_buf & operator&=(T const b) noexcept {
    return *this = sst::to_unsigned(static_cast<value_type>(*this))
                   & sst::to_unsigned(b);
  }

  template<
      class OtherSizeType,
      class OtherValueType,
      bool OtherAligned,
      bool OtherReversed,
      class V = ValueType,
      typename std::enable_if<!std::is_const<V>::value, int>::type = 0>
  uint_buf & operator=(uint_buf<OtherSizeType,
                                OtherValueType,
                                OtherAligned,
                                OtherReversed> const & x) noexcept {
    return *this = static_cast<value_type>(x);
  }

  template<
      class OtherSizeType,
      class OtherValueType,
      bool OtherAligned,
      bool OtherReversed,
      class V = ValueType,
      typename std::enable_if<!std::is_const<V>::value, int>::type = 0>
  uint_buf & operator+=(uint_buf<OtherSizeType,
                                 OtherValueType,
                                 OtherAligned,
                                 OtherReversed> const & x) noexcept {
    return *this += static_cast<value_type>(x);
  }

  template<
      class OtherSizeType,
      class OtherValueType,
      bool OtherAligned,
      bool OtherReversed,
      class V = ValueType,
      typename std::enable_if<!std::is_const<V>::value, int>::type = 0>
  uint_buf & operator*=(uint_buf<OtherSizeType,
                                 OtherValueType,
                                 OtherAligned,
                                 OtherReversed> const & x) noexcept {
    return *this *= static_cast<value_type>(x);
  }

  template<
      class OtherSizeType,
      class OtherValueType,
      bool OtherAligned,
      bool OtherReversed,
      class V = ValueType,
      typename std::enable_if<!std::is_const<V>::value, int>::type = 0>
  uint_buf & operator&=(uint_buf<OtherSizeType,
                                 OtherValueType,
                                 OtherAligned,
                                 OtherReversed> const & x) noexcept {
    return *this &= static_cast<value_type>(x);
  }
};

template<class SizeType = std::size_t, class = void>
class uint_buf_iterator final {

  unsigned char * buf_;
  SizeType size_;
  bool little_endian_;

public:
  explicit uint_buf_iterator(void const * const buf,
                             SizeType const size,
                             bool const little_endian)
      : buf_(
          (assert(buf != nullptr),
           assert(size > 0),
           reinterpret_cast<unsigned char *>(const_cast<void *>(buf)))),
        size_(size),
        little_endian_(little_endian) {
  }

  uint_buf<SizeType> operator*() noexcept {
    return uint_buf<SizeType>(buf_, size_, little_endian_);
  }

  uint_buf<SizeType> const operator*() const noexcept {
    return uint_buf<SizeType>(buf_, size_, little_endian_);
  }

  uint_buf_iterator & operator++() {
    buf_ += size_;
    return *this;
  }

  uint_buf_iterator operator++(int) {
    auto x = *this;
    ++*this;
    return x;
  }
};

template<class UintBufType>
class uint_buf_add_eq final {
  UintBufType p_;

  void increment() noexcept {
    p_.increment();
  }

  template<class, class>
  friend class uint_buf_iterator;

public:
  template<class... Args>
  explicit uint_buf_add_eq(Args &&... args)
      : p_(std::forward<Args>(args)...) {
  }

  ~uint_buf_add_eq() noexcept = default;
  uint_buf_add_eq(uint_buf_add_eq const &) noexcept = default;
  uint_buf_add_eq &
  operator=(uint_buf_add_eq const &) noexcept = default;
  uint_buf_add_eq(uint_buf_add_eq &&) noexcept = default;
  uint_buf_add_eq & operator=(uint_buf_add_eq &&) noexcept = default;

  template<class Arg>
  uint_buf_add_eq & operator=(Arg && arg) {
    p_ += std::forward<Arg>(arg);
    return *this;
  }
};

template<class T, class UintBufType>
class dot_accumulator final {
  T * dot_;
  UintBufType p_;

  void increment() noexcept {
    p_.increment();
  }

  template<class, class>
  friend class uint_buf_iterator;

public:
  template<class... Args>
  explicit dot_accumulator(T * const dot, Args &&... args)
      : dot_((assert(dot != nullptr), dot)),
        p_(std::forward<Args>(args)...) {
  }

  ~dot_accumulator() noexcept = default;
  dot_accumulator(dot_accumulator const &) noexcept = default;
  dot_accumulator &
  operator=(dot_accumulator const &) noexcept = default;
  dot_accumulator(dot_accumulator &&) noexcept = default;
  dot_accumulator & operator=(dot_accumulator &&) noexcept = default;

  template<class Arg,
           typename std::enable_if<std::is_integral<Arg>::value,
                                   int>::type = 0>
  dot_accumulator & operator=(Arg const arg) {
    *dot_ = T(sst::to_unsigned(*dot_) + p_ * arg);
    return *this;
  }

  template<class Arg,
           typename std::enable_if<std::is_integral<Arg>::value,
                                   int>::type = 0>
  dot_accumulator const & operator=(Arg const arg) const {
    *dot_ = T(sst::to_unsigned(*dot_) + p_ * arg);
    return *this;
  }
};

template<class UintBufType>
class uint_buf_and_eq final {
  UintBufType p_;

  void increment() noexcept {
    p_.increment();
  }

  template<class, class>
  friend class uint_buf_iterator;

public:
  template<class... Args>
  explicit uint_buf_and_eq(Args &&... args)
      : p_(std::forward<Args>(args)...) {
  }

  ~uint_buf_and_eq() noexcept = default;
  uint_buf_and_eq(uint_buf_and_eq const &) noexcept = default;
  uint_buf_and_eq &
  operator=(uint_buf_and_eq const &) noexcept = default;
  uint_buf_and_eq(uint_buf_and_eq &&) noexcept = default;
  uint_buf_and_eq & operator=(uint_buf_and_eq &&) noexcept = default;

  template<class Arg>
  uint_buf_and_eq & operator=(Arg && arg) {
    p_ &= std::forward<Arg>(arg);
    return *this;
  }
};

template<class UintBufType>
class uint_buf_iterator<UintBufType,
                        typename std::enable_if<!std::is_integral<
                            UintBufType>::value>::type>
    final {

  UintBufType p_;

public:
  template<class... Args>
  explicit uint_buf_iterator(Args &&... args)
      : p_(std::forward<Args>(args)...) {
  }

  UintBufType operator*() const noexcept {
    return p_;
  }

  uint_buf_iterator & operator++() {
    p_.increment();
    return *this;
  }

  uint_buf_iterator operator++(int) {
    auto const x = *this;
    ++*this;
    return x;
  }
};

template<class RawType, class uint_T>
void raw_from_uint_general(RawType & raw, uint_T const x) noexcept {
  PFSS_STATIC_ASSERT(is_exact_width_integer<uint_T>::value);
  PFSS_STATIC_ASSERT(std::is_unsigned<uint_T>::value);
  PFSS_STATIC_ASSERT(sizeof(uint_T) <= sizeof(RawType));
  PFSS_STATIC_ASSERT(is_little_endian<uint_T>::value
                     || is_big_endian<uint_T>::value);
  auto const src = reinterpret_cast<unsigned char const *>(&x);
  auto const dst = reinterpret_cast<unsigned char *>(&raw);
  if (is_little_endian<uint_T>::value) {
    std::copy(src, src + sizeof(uint_T), dst);
  } else {
    std::reverse_copy(src, src + sizeof(uint_T), dst);
  }
  std::fill(dst + sizeof(uint_T),
            dst + sizeof(RawType),
            static_cast<unsigned char>(0));
}

//----------------------------------------------------------------------
// pfss::chunked_block
//----------------------------------------------------------------------

template<class ChunkType, int ChunkCount>
class chunked_block final {

  using chunk_type = typename remove_cvref<ChunkType>::type;
  PFSS_STATIC_ASSERT(is_unsigned_integer<chunk_type>());
  static constexpr int chunk_bits = type_bits<chunk_type>();
  PFSS_STATIC_ASSERT(chunk_bits % uchar_bits == 0);
  PFSS_STATIC_ASSERT(chunk_bits / uchar_bits == sizeof(chunk_type));
  PFSS_STATIC_ASSERT(ChunkCount > 0);
  PFSS_STATIC_ASSERT(chunk_bits <= type_max<int>() / ChunkCount);

  //
  // We need to make sure that raw_ doesn't have any padding in order to
  // satisfy the raw_type requirements, so we need to use a plain array
  // instead of std::array.
  //

  chunk_type raw_[ChunkCount];

public:
  static constexpr int bits = chunk_bits * ChunkCount;
  static constexpr int bytes = bits / uchar_bits;

  template<class uint_T>
  static chunked_block from_uint(uint_T x) noexcept {
    chunked_block b;
    raw_from_uint_general(b.raw_, x);
    return b;
  }

  //--------------------------------------------------------------------
  // Raw access
  //--------------------------------------------------------------------
  //
  // raw_type is suitable for storing multiple blocks into an array and
  // accessing them as one big blob of bytes.
  //

  using raw_type = decltype(raw_);

  raw_type & raw() noexcept {
    return raw_;
  }

  raw_type const & raw() const noexcept {
    return raw_;
  }

  //--------------------------------------------------------------------
  // Container access
  //--------------------------------------------------------------------

  using size_type = unsigned int;

  size_type size() const noexcept {
    return static_cast<size_type>(bytes);
  }

  unsigned char & operator[](size_type const i) noexcept {
    assert(i < size());
    return reinterpret_cast<unsigned char *>(&raw_)[i];
  }

  unsigned char const & operator[](size_type const i) const noexcept {
    assert(i < size());
    return reinterpret_cast<unsigned char const *>(&raw_)[i];
  }

  //--------------------------------------------------------------------
  // MSB access
  //--------------------------------------------------------------------

  bool get_msb() const noexcept {
    return static_cast<bool>((*this)[size() - 1] & uchar_msb);
  }

  chunked_block set_msb(bool const b) const noexcept {
    chunked_block x = *this;
    if (b) {
      x[x.size() - 1] |= uchar_msb;
    } else {
      x[x.size() - 1] &= uchar_msb_neg;
    }
    return x;
  }

  chunked_block flip_msb() const noexcept {
    chunked_block x = *this;
    x[x.size() - 1] ^= uchar_msb;
    return x;
  }

  //--------------------------------------------------------------------
  // Arithmetic
  //--------------------------------------------------------------------

  chunked_block & operator^=(chunked_block const & rhs) noexcept {
    for (int i = 0; i != ChunkCount; ++i) {
      raw_[i] ^= rhs.raw_[i];
    }
    return *this;
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class OutputIt>
  OutputIt serialize(OutputIt out) const {
    for (size_type i = 0; i != size(); ++i) {
      *out++ = (*this)[i];
    }
    return out;
  }

  template<class InputIt>
  InputIt parse(InputIt in) {
    for (size_type i = 0; i != size(); ++i) {
      (*this)[i] = *in++;
    }
    return in;
  }
};

template<class ChunkType, int ChunkCount>
chunked_block<ChunkType, ChunkCount> operator^(
    chunked_block<ChunkType, ChunkCount> lhs,
    chunked_block<ChunkType, ChunkCount> const & rhs) noexcept {
  lhs ^= rhs;
  return lhs;
}

using basic_block = chunked_block<std::uint64_t, 2>;

//----------------------------------------------------------------------
// pfss::m128i_block
//----------------------------------------------------------------------

#if PFSS_HAVE_SSE2

class m128i_block final {

#if PFSS_HAVE_AES_NI
  template<bool Aligned>
  friend class aes_ni_128_rand_perm;
#endif

  __m128i raw_;

public:
  static constexpr int bits = 128;
  static constexpr int bytes = 16;

  m128i_block() noexcept = default;

  m128i_block(__m128i const x) noexcept : raw_(x) {
  }

  operator __m128i() const noexcept {
    return raw_;
  }

  template<class uint_T>
  static m128i_block from_uint(uint_T x) noexcept {
    PFSS_STATIC_ASSERT(is_exact_width_integer<uint_T>::value);
    PFSS_STATIC_ASSERT(std::is_unsigned<uint_T>::value);
    PFSS_STATIC_ASSERT(sizeof(uint_T) <= sizeof(m128i_block));
    PFSS_STATIC_ASSERT(   is_little_endian<uint_T>::value);
    PFSS_STATIC_ASSERT(value_bits<uint_T>::value <= 64);
    m128i_block b;
    b.raw() = _mm_set_epi64x(0, x);
    return b;
  }

  //--------------------------------------------------------------------
  // Raw access
  //--------------------------------------------------------------------
  //
  // raw_type is suitable for storing multiple blocks into an array and
  // accessing them as one big blob of bytes.
  //

  using raw_type = decltype(raw_);

  raw_type & raw() noexcept {
    return raw_;
  }

  raw_type const & raw() const noexcept {
    return raw_;
  }

  //--------------------------------------------------------------------
  // Container access
  //--------------------------------------------------------------------

  using size_type = unsigned int;

  size_type size() const noexcept {
    return static_cast<size_type>(bytes);
  }

  unsigned char & operator[](size_type const i) noexcept {
    assert(i < size());
    return reinterpret_cast<unsigned char *>(&raw_)[i];
  }

  unsigned char const & operator[](size_type const i) const noexcept {
    assert(i < size());
    return reinterpret_cast<unsigned char const *>(&raw_)[i];
  }

  //--------------------------------------------------------------------
  // MSB access
  //--------------------------------------------------------------------

  bool get_msb() const noexcept {
    return static_cast<bool>(_mm_movemask_epi8(raw_) & 32768);
  }

  m128i_block set_msb(bool const b) const noexcept {
    if (b) {
      return _mm_or_si128(raw_, _mm_set_epi32(INT_MIN, 0, 0, 0));
    } else {
      return _mm_and_si128(raw_, _mm_set_epi32(INT_MAX, -1, -1, -1));
    }
  }

  m128i_block flip_msb() const noexcept {
    return _mm_xor_si128(raw_, _mm_set_epi32(INT_MIN, 0, 0, 0));
  }

  //--------------------------------------------------------------------
  // Arithmetic
  //--------------------------------------------------------------------

  m128i_block operator^(m128i_block const & other) const noexcept {
    return _mm_xor_si128(raw_, other.raw_);
  }

  m128i_block & operator^=(m128i_block const & other) noexcept {
    return *this = *this ^ other;
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class OutputIt>
  OutputIt serialize(OutputIt out) const {
    for (size_type i = 0; i != size(); ++i) {
      *out++ = (*this)[i];
    }
    return out;
  }

  template<class InputIt>
  InputIt parse(InputIt in) {
    for (size_type i = 0; i != size(); ++i) {
      (*this)[i] = *in++;
    }
    return in;
  }
};

#endif // PFSS_HAVE_SSE2

//----------------------------------------------------------------------
// pfss::m512i_block
//----------------------------------------------------------------------

#if PFSS_HAVE_AVX_512_F

class m512i_block final {

  __m512i raw_;

public:

  m512i_block() noexcept = default;

  m512i_block(__m512i const & x) noexcept : raw_(x) {
  }

  operator __m512i() const noexcept {
    return raw_;
  }

  m512i_block operator^(m512i_block const & other) const noexcept {
    return _mm512_xor_si512(raw_, other.raw_);
  }

  m512i_block & operator^=(m512i_block const & other) noexcept {
    return *this = *this ^ other;
  }
};

#endif // PFSS_HAVE_AVX_512_F

//----------------------------------------------------------------------
// pfss::uint8x16_block
//----------------------------------------------------------------------

#if PFSS_HAVE_ARM_CRYPTO

class uint8x16_block final {

  uint8x16_t raw_;

public:
  static constexpr int bits = 128;
  static constexpr int bytes = 16;

  uint8x16_block() noexcept = default;

  uint8x16_block(uint8x16_t const x) noexcept : raw_(x) {
  }

  operator uint8x16_t() const noexcept {
    return raw_;
  }

  template<class uint_T>
  static uint8x16_block from_uint(uint_T x) noexcept {
    uint8x16_block b;
    raw_from_uint_general(b.raw_, x);
    return b;
  }

  //--------------------------------------------------------------------
  // Raw access
  //--------------------------------------------------------------------
  //
  // raw_type is suitable for storing multiple blocks into an array and
  // accessing them as one big blob of bytes.
  //

  using raw_type = decltype(raw_);

  raw_type & raw() noexcept {
    return raw_;
  }

  raw_type const & raw() const noexcept {
    return raw_;
  }

  //--------------------------------------------------------------------
  // Container access
  //--------------------------------------------------------------------

  using size_type = unsigned int;

  size_type size() const noexcept {
    return static_cast<size_type>(bytes);
  }

  unsigned char & operator[](size_type const i) noexcept {
    assert(i < size());
    return reinterpret_cast<unsigned char *>(&raw_)[i];
  }

  unsigned char const & operator[](size_type const i) const noexcept {
    assert(i < size());
    return reinterpret_cast<unsigned char const *>(&raw_)[i];
  }

  //--------------------------------------------------------------------
  // MSB access
  //--------------------------------------------------------------------

  bool get_msb() const noexcept {
    return (*this)[size() - 1] & uchar_msb;
  }

  uint8x16_block set_msb(bool const b) const noexcept {
    uint8x16_block x = *this;
    if (b) {
      x[x.size() - 1] |= uchar_msb;
    } else {
      x[x.size() - 1] &= uchar_msb_neg;
    }
    return x;
  }

  uint8x16_block flip_msb() const noexcept {
    uint8x16_block x = *this;
    x[x.size() - 1] ^= uchar_msb;
    return x;
  }

  //--------------------------------------------------------------------
  // Arithmetic
  //--------------------------------------------------------------------

  uint8x16_block & operator^=(uint8x16_block const & rhs) noexcept {
    raw_ ^= rhs.raw_;
    return *this;
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class OutputIt>
  OutputIt serialize(OutputIt out) const {
    for (size_type i = 0; i != size(); ++i) {
      *out++ = (*this)[i];
    }
    return out;
  }

  template<class InputIt>
  InputIt parse(InputIt in) {
    for (size_type i = 0; i != size(); ++i) {
      (*this)[i] = *in++;
    }
    return in;
  }
};

inline uint8x16_block
operator^(uint8x16_block lhs, uint8x16_block const & rhs) noexcept {
  lhs ^= rhs;
  return lhs;
}

#endif // PFSS_HAVE_ARM_CRYPTO

//----------------------------------------------------------------------
// default_block_t
//----------------------------------------------------------------------

namespace detail {

template<int Bits>
struct default_block_t {};

template<>
struct default_block_t<128> {
#if PFSS_HAVE_AES_NI
  using type = m128i_block;
#elif PFSS_HAVE_ARM_CRYPTO
  using type = uint8x16_block;
#elif PFSS_HAVE_SSE2
  using type = m128i_block;
#elif PFSS_HAVE_UINT64
  using type = chunked_block<uint64_t, 2>;
#endif
};

} // namespace detail

template<int Bits = 128>
using default_block_t = typename detail::default_block_t<Bits>::type;

//----------------------------------------------------------------------
//
//

template<class T, class SizeType = std::size_t>
class buffered_rng final {

  PFSS_STATIC_ASSERT(std::is_trivial<T>::value);

  unsigned char const * buf;

  using size_type = typename promote_unsigned<SizeType>::type;
  size_type const size;

public:
  explicit constexpr buffered_rng(
      unsigned char const * const buf, size_type const buf_size)
      : buf(buf), size(buf_size) {
    PFSS_CONSTEXPR_ASSERT(buf != nullptr);
    PFSS_CONSTEXPR_ASSERT(buf_size != 0);
  }

  ~buffered_rng() noexcept = default;

  buffered_rng(buffered_rng const &) = delete;
  buffered_rng(buffered_rng &&) = delete;
  buffered_rng & operator=(buffered_rng const &) = delete;
  buffered_rng & operator=(buffered_rng &&) = delete;

  T operator()() {
    T x;
    assert(sizeof(x) <= size);
    std::copy_n(buf, sizeof(x), reinterpret_cast<unsigned char *>(&x));
    buf += sizeof(x);
    return x;
  }
};

//
// basic_rng uses C++'s <random> facilities.
//

template<class Block>
class basic_rng final {
  std::random_device rd;
  std::uniform_int_distribution<unsigned char> dist;

public:
  basic_rng() : dist(0, type_max<unsigned char>()) {
  }
  Block operator()() {
    Block b;
    auto const n = b.size();
    for (decltype(+n) i = 0; i != n; ++i) {
      b[i] = dist(rd);
    }
    return b;
  }
};

//
// C++'s <random> with std::random_device("/dev/urandom").
//

template<class Block>
class cpp_urandom_rng final {
  std::random_device rd;
  std::uniform_int_distribution<unsigned char> dist;

public:
  cpp_urandom_rng()
      : rd("/dev/urandom"), dist(0, type_max<unsigned char>()) {
  }
  Block operator()() {
    Block b;
    auto const n = b.size();
    for (decltype(+n) i = 0; i != n; ++i) {
      b[i] = dist(rd);
    }
    return b;
  }
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------

namespace fips197 {

constexpr int Nb = 4;

constexpr unsigned char Rcon[] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

constexpr unsigned char Sbox[] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67,
    0x2B, 0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59,
    0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7,
    0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1,
    0x71, 0xD8, 0x31, 0x15, 0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05,
    0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83,
    0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29,
    0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B,
    0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF, 0xD0, 0xEF, 0xAA,
    0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C,
    0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC,
    0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
    0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19,
    0x73, 0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE,
    0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49,
    0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4,
    0xEA, 0x65, 0x7A, 0xAE, 0x08, 0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6,
    0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A, 0x70,
    0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9,
    0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E,
    0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF, 0x8C, 0xA1,
    0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0,
    0x54, 0xBB, 0x16,
};

template<class W>
void RotWord(W w) {
  auto const temp = w[0];
  for (int j = 1; j < Nb; ++j) {
    w[j - 1] = w[j];
  }
  w[Nb - 1] = temp;
}

template<class W>
void SubWord(W w) {
  for (int j = 0; j < Nb; ++j) {
    w[j] = Sbox[w[j]];
  }
}

template<int Nk, class Key, class W>
void KeyExpansion(Key key, W w) {
  PFSS_STATIC_ASSERT(Nk == 4 || Nk == 6 || Nk == 8);
  constexpr int Nr = Nk + 6;
  unsigned char temp[Nb];
  int i = 0;
  for (; i < Nk; ++i) {
    for (int j = 0; j < Nb; ++j) {
      w[i * Nb + j] = key[i * Nb + j];
    }
  }
  for (; i < Nb * (Nr + 1); ++i) {
    for (int j = 0; j < Nb; ++j) {
      temp[j] = w[(i - 1) * Nb + j];
    }
    if (i % Nk == 0) {
      RotWord(temp);
      SubWord(temp);
      temp[0] ^= Rcon[i / Nk];
    } else if (Nk > 6 && i % Nk == 4) {
      SubWord(temp);
    }
    for (int j = 0; j < Nb; ++j) {
      w[i * Nb + j] = w[(i - Nk) * Nb + j] ^ temp[j];
    }
  }
}

} // namespace fips197

//----------------------------------------------------------------------
// Fixed keys
//----------------------------------------------------------------------

constexpr std::array<unsigned char, 16> fixed_aes_128_key =
#include <pfss/fixed_aes_128_key.inc>
    ;

constexpr std::array<unsigned char, 176> fixed_aes_128_rkeys = {
#include <pfss/fixed_aes_128_rkeys.inc>
};

// clang-format off
constexpr std::array<unsigned char, 704> fixed_aes_128_rkeys_x4 = {
#include <pfss/fixed_aes_128_rkeys.inc>
#include <pfss/fixed_aes_128_rkeys.inc>
#include <pfss/fixed_aes_128_rkeys.inc>
#include <pfss/fixed_aes_128_rkeys.inc>
};
// clang-format on

//----------------------------------------------------------------------
// rand_perm_nettle_aes128
//----------------------------------------------------------------------

#if PFSS_HAVE_NETTLE

template<class Block>
Block nettle_aes_128_encrypt(
    Block block, aes128_ctx const & rkeys) noexcept {
  PFSS_STATIC_ASSERT(sizeof(Block) == 16);
  auto const p = reinterpret_cast<unsigned char *>(&block);
  aes128_encrypt(&rkeys, 16, p, p);
  return block;
}

template<class BlockType>
class rand_perm_nettle_aes128 final {

  struct aes128_ctx ctx;

public:
  using block_type = BlockType;
  PFSS_STATIC_ASSERT(block_type::bits == 128);

  rand_perm_nettle_aes128(void const * const key) noexcept {
    assert(key != nullptr);
    aes128_set_encrypt_key(&ctx, static_cast<uint8_t const *>(key));
  }

  ~rand_perm_nettle_aes128() = default;
  rand_perm_nettle_aes128(rand_perm_nettle_aes128 const &) = default;
  rand_perm_nettle_aes128(rand_perm_nettle_aes128 &&) = default;
  rand_perm_nettle_aes128 &
  operator=(rand_perm_nettle_aes128 const &) = default;
  rand_perm_nettle_aes128 &
  operator=(rand_perm_nettle_aes128 &&) = default;

  block_type operator()(block_type x) {
    return nettle_aes_128_encrypt(x, ctx);
  }
};

constexpr std::array<unsigned char, 176>
    fixed_nettle_aes_128_rkeys_data alignas(aes128_ctx) =
        fixed_aes_128_rkeys;

static aes128_ctx const & fixed_nettle_aes_128_rkeys =
    *reinterpret_cast<aes128_ctx const *>(
        &fixed_nettle_aes_128_rkeys_data);

template<class Block>
struct fixed_nettle_aes_128_rand_perm final {
  Block operator()(Block const block) const noexcept {
    return nettle_aes_128_encrypt(block, fixed_nettle_aes_128_rkeys);
  }
};

#endif // PFSS_HAVE_NETTLE

//----------------------------------------------------------------------
// aes_ni_128_encrypt
//----------------------------------------------------------------------

#if PFSS_HAVE_AES_NI

inline __m128i aes_ni_128_encrypt(
    __m128i block, __m128i const * const rkeys) noexcept {
  assert(rkeys != nullptr);
  block = _mm_xor_si128(block, rkeys[0]);
  block = _mm_aesenc_si128(block, rkeys[1]);
  block = _mm_aesenc_si128(block, rkeys[2]);
  block = _mm_aesenc_si128(block, rkeys[3]);
  block = _mm_aesenc_si128(block, rkeys[4]);
  block = _mm_aesenc_si128(block, rkeys[5]);
  block = _mm_aesenc_si128(block, rkeys[6]);
  block = _mm_aesenc_si128(block, rkeys[7]);
  block = _mm_aesenc_si128(block, rkeys[8]);
  block = _mm_aesenc_si128(block, rkeys[9]);
  block = _mm_aesenclast_si128(block, rkeys[10]);
  return block;
}

#endif // PFSS_HAVE_AES_NI

//----------------------------------------------------------------------
// aes_ni_128_rand_perm
//----------------------------------------------------------------------

#if PFSS_HAVE_AES_NI

template<bool Aligned = false>
class aes_ni_128_rand_perm final {

  // Round keys.
  __m128i w[11];

  // Helper function for key expansion.
  static __m128i helper(__m128i const a, __m128i const b) noexcept {
    __m128i c = a;
    c = _mm_xor_si128(c, _mm_slli_si128(a, 4));
    c = _mm_xor_si128(c, _mm_slli_si128(a, 8));
    c = _mm_xor_si128(c, _mm_slli_si128(a, 12));
    c = _mm_xor_si128(c, _mm_shuffle_epi32(b, 255));
    return c;
  }

  // Expands a key into the round keys.
  void expand(__m128i const k) noexcept {
    w[0] = k;
    w[1] = helper(w[0], _mm_aeskeygenassist_si128(w[0], 1));
    w[2] = helper(w[1], _mm_aeskeygenassist_si128(w[1], 2));
    w[3] = helper(w[2], _mm_aeskeygenassist_si128(w[2], 4));
    w[4] = helper(w[3], _mm_aeskeygenassist_si128(w[3], 8));
    w[5] = helper(w[4], _mm_aeskeygenassist_si128(w[4], 16));
    w[6] = helper(w[5], _mm_aeskeygenassist_si128(w[5], 32));
    w[7] = helper(w[6], _mm_aeskeygenassist_si128(w[6], 64));
    w[8] = helper(w[7], _mm_aeskeygenassist_si128(w[7], 128));
    w[9] = helper(w[8], _mm_aeskeygenassist_si128(w[8], 27));
    w[10] = helper(w[9], _mm_aeskeygenassist_si128(w[9], 54));
  }

  // Encrypts a block using the round keys.
  __m128i encrypt(__m128i x) const noexcept {
    return aes_ni_128_encrypt(x, w);
  }

public:
  using block_type = m128i_block;

  aes_ni_128_rand_perm(void const * const key) noexcept {
    assert(key != nullptr);
    __m128i k;
    if (Aligned) {
      k = _mm_load_si128(static_cast<__m128i const *>(key));
    } else {
      k = _mm_loadu_si128(static_cast<__m128i const *>(key));
    }
    expand(k);
  }

  ~aes_ni_128_rand_perm() noexcept = default;
  aes_ni_128_rand_perm(aes_ni_128_rand_perm const &) noexcept = default;
  aes_ni_128_rand_perm(aes_ni_128_rand_perm &&) noexcept = default;
  aes_ni_128_rand_perm &
  operator=(aes_ni_128_rand_perm const &) noexcept = default;
  aes_ni_128_rand_perm &
  operator=(aes_ni_128_rand_perm &&) noexcept = default;

  block_type operator()(block_type const x) const noexcept {
    return encrypt(x);
  }
};

#endif // PFSS_HAVE_AES_NI

//----------------------------------------------------------------------
// fixed_aes_ni_128_rand_perm
//----------------------------------------------------------------------

#if PFSS_HAVE_AES_NI

constexpr std::array<unsigned char, 176>
    fixed_aes_ni_128_rkeys_data alignas(__m128i) = fixed_aes_128_rkeys;

static __m128i const * const fixed_aes_ni_128_rkeys =
    reinterpret_cast<__m128i const *>(&fixed_aes_ni_128_rkeys_data);

struct fixed_aes_ni_128_rand_perm final {
  m128i_block operator()(m128i_block const block) const noexcept {
    return aes_ni_128_encrypt(block, fixed_aes_ni_128_rkeys);
  }
};

#endif // PFSS_HAVE_AES_NI

//----------------------------------------------------------------------
// arm_crypto_aes_128_rand_perm
//----------------------------------------------------------------------

#if PFSS_HAVE_ARM_CRYPTO

inline uint8x16_t arm_aes_128_encrypt(
    uint8x16_t block, uint8x16_t const * const rkeys) noexcept {
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[0]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[1]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[2]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[3]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[4]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[5]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[6]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[7]));
  block = vaesmcq_u8(vaeseq_u8(block, rkeys[8]));
  block = veorq_u8(vaeseq_u8(block, rkeys[9]), rkeys[10]);
  return block;
}

class arm_crypto_aes_128_rand_perm final {

  // Round keys.
  uint8x16_t w[11];

  // Encrypts a block using the round keys.
  uint8x16_t encrypt(uint8x16_t x) const noexcept {
    return arm_aes_128_encrypt(x, w);
  }

public:
  using block_type = uint8x16_block;

  arm_crypto_aes_128_rand_perm(void const * const key) noexcept {
    assert(key != nullptr);
    fips197::KeyExpansion<4>(
        static_cast<unsigned char const *>(key),
        reinterpret_cast<unsigned char *>(&w));
  }

  ~arm_crypto_aes_128_rand_perm() noexcept = default;
  arm_crypto_aes_128_rand_perm(
      arm_crypto_aes_128_rand_perm const &) noexcept = default;
  arm_crypto_aes_128_rand_perm(
      arm_crypto_aes_128_rand_perm &&) noexcept = default;
  arm_crypto_aes_128_rand_perm &
  operator=(arm_crypto_aes_128_rand_perm const &) noexcept = default;
  arm_crypto_aes_128_rand_perm &
  operator=(arm_crypto_aes_128_rand_perm &&) noexcept = default;

  block_type operator()(block_type const x) const noexcept {
    return encrypt(x);
  }
};

constexpr std::array<unsigned char, 176>
    fixed_arm_aes_128_rkeys_data alignas(uint8x16_t) =
        fixed_aes_128_rkeys;

static uint8x16_t const * const fixed_arm_aes_128_rkeys =
    reinterpret_cast<uint8x16_t const *>(&fixed_arm_aes_128_rkeys_data);

struct fixed_arm_aes_128_rand_perm final {
  uint8x16_block operator()(uint8x16_block const block) const noexcept {
    return arm_aes_128_encrypt(block, fixed_arm_aes_128_rkeys);
  }
};

#endif // PFSS_HAVE_ARM_CRYPTO

//----------------------------------------------------------------------
// default_rand_perm_t
//----------------------------------------------------------------------

namespace detail {

template<class Block>
struct default_rand_perm_t {};

#if PFSS_HAVE_UINT64
template<>
struct default_rand_perm_t<chunked_block<uint64_t, 2>> {
#if PFSS_HAVE_NETTLE
  using type =
      fixed_nettle_aes_128_rand_perm<chunked_block<uint64_t, 2>>;
#endif
};
#endif

#if PFSS_HAVE_SSE2
template<>
struct default_rand_perm_t<m128i_block> {
#if PFSS_HAVE_AES_NI
  using type = fixed_aes_ni_128_rand_perm;
#elif PFSS_HAVE_NETTLE
  using type = fixed_nettle_aes_128_rand_perm<m128i_block>;
#endif
};
#endif

#if PFSS_HAVE_ARM_CRYPTO
template<>
struct default_rand_perm_t<uint8x16_block> {
  using type = fixed_arm_aes_128_rand_perm;
};
#endif

} // namespace detail

template<class Block = default_block_t<>>
using default_rand_perm_t =
    typename detail::default_rand_perm_t<Block>::type;

//----------------------------------------------------------------------
// default_rng_t
//----------------------------------------------------------------------

template<class Block = default_block_t<>>
using default_rng_t = cpp_urandom_rng<Block>;

//----------------------------------------------------------------------

} // namespace pfss

#endif // PFSS_COMMON_H
