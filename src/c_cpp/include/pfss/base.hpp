//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BASE_HPP
#define PFSS_BASE_HPP

#include <cassert>
#include <chrono>
#include <cstddef>
#include <limits>
#include <mutex>
#include <pfss.h>
#include <pfss/config.h>
#include <stdexcept>
#include <type_traits>

#if PFSS_HAVE_X86
#include <x86intrin.h>
#endif

namespace sst {

//----------------------------------------------------------------------
// __cplusplus values
//----------------------------------------------------------------------

#define PFSS_CPP97_VALUE 199711L
#define PFSS_CPP03_VALUE PFSS_CPP97_VALUE
#define PFSS_CPP11_VALUE 201103L
#define PFSS_CPP14_VALUE 201402L
#define PFSS_CPP17_VALUE 201703L
#define PFSS_CPP20_VALUE 202002L

//----------------------------------------------------------------------
// __cplusplus comparisons
//----------------------------------------------------------------------

#define PFSS_CPP97_OR_LATER (__cplusplus >= PFSS_CPP97_VALUE)
#define PFSS_CPP03_OR_LATER (__cplusplus >= PFSS_CPP03_VALUE)
#define PFSS_CPP11_OR_LATER (__cplusplus >= PFSS_CPP11_VALUE)
#define PFSS_CPP14_OR_LATER (__cplusplus >= PFSS_CPP14_VALUE)
#define PFSS_CPP17_OR_LATER (__cplusplus >= PFSS_CPP17_VALUE)
#define PFSS_CPP20_OR_LATER (__cplusplus >= PFSS_CPP20_VALUE)

#if !PFSS_CPP11_OR_LATER
#error PFSS requires C++11 or later
#endif

//----------------------------------------------------------------------
// PFSS_STATIC_ASSERT
//----------------------------------------------------------------------

#if PFSS_CPP17_OR_LATER
#define PFSS_STATIC_ASSERT(x) static_assert((x))
#else
#define PFSS_STATIC_ASSERT(x) static_assert((x), #x)
#endif

//----------------------------------------------------------------------
// PFSS_CONSTEXPR_ASSERT
//----------------------------------------------------------------------

#if PFSS_CPP14_OR_LATER
#define PFSS_CONSTEXPR_ASSERT(x) assert((x))
#else
#define PFSS_CONSTEXPR_ASSERT(x)
#endif

//----------------------------------------------------------------------
// PFSS_CPP14_CONSTEXPR
//----------------------------------------------------------------------

#if PFSS_CPP14_OR_LATER
#define PFSS_CPP14_CONSTEXPR constexpr
#else
#define PFSS_CPP14_CONSTEXPR
#endif

//----------------------------------------------------------------------
// sst::value_bits
//----------------------------------------------------------------------

template<class T>
struct value_bits
    : std::integral_constant<int, std::numeric_limits<T>::digits> {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
};

//----------------------------------------------------------------------
// sst::width_bits
//----------------------------------------------------------------------

template<class IntType, class = void>
struct width_bits;

template<class IntType>
struct width_bits<
    IntType,
    typename std::enable_if<std::is_integral<IntType>::value>::type>
    : std::integral_constant<int,
                             sst::value_bits<IntType>::value
                                 + static_cast<int>(
                                     std::is_signed<IntType>::value)> {
};

//----------------------------------------------------------------------
// sst::type_min
//----------------------------------------------------------------------

template<class T>
struct type_min
    : std::integral_constant<T, std::numeric_limits<T>::min()> {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
};

//----------------------------------------------------------------------
// sst::type_max
//----------------------------------------------------------------------

template<class T>
struct type_max
    : std::integral_constant<T, std::numeric_limits<T>::max()> {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
};

//----------------------------------------------------------------------
// sst::remove_cvref
//----------------------------------------------------------------------

template<class T>
struct remove_cvref {
  using type = typename std::remove_cv<
      typename std::remove_reference<T>::type>::type;
};

//----------------------------------------------------------------------
// sst::integer_promote
//----------------------------------------------------------------------

template<class T>
class integer_promote {
  PFSS_STATIC_ASSERT(std::is_integral<T>());
  using T1 = decltype(+T());
  using T2 = typename std::
      conditional<+std::is_const<T>(), T1 const, T1>::type;
  using T3 = typename std::
      conditional<+std::is_volatile<T>(), T2 volatile, T2>::type;

public:
  using type = T3;
};

//----------------------------------------------------------------------
// sst::is_signed_integer
//----------------------------------------------------------------------

template<class, class = void>
struct is_signed_integer : std::false_type {};

template<class T>
struct is_signed_integer<
    T,
    typename std::enable_if<std::is_signed<T>::value
                            && std::is_integral<T>::value>::type>
    : std::true_type {};

//----------------------------------------------------------------------
// sst::is_unsigned_integer
//----------------------------------------------------------------------

template<class, class = void>
struct is_unsigned_integer : std::false_type {};

template<class T>
struct is_unsigned_integer<
    T,
    typename std::enable_if<std::is_unsigned<T>::value
                            && std::is_integral<T>::value>::type>
    : std::true_type {};

//----------------------------------------------------------------------
// sst::is_twos_complement
//----------------------------------------------------------------------

template<class T = int, class = void>
struct is_twos_complement : std::false_type {};

template<class T>
struct is_twos_complement<
    T,
    typename std::enable_if<is_unsigned_integer<T>()
                            || (is_signed_integer<T>()
                                && ~1 == -2)>::type> : std::true_type {
};

//----------------------------------------------------------------------
// sst::is_ones_complement
//----------------------------------------------------------------------

template<class T = int, class = void>
struct is_ones_complement : std::false_type {};

template<class T>
struct is_ones_complement<
    T,
    typename std::enable_if<is_signed_integer<T>() && ~1 == -1>::type>
    : std::true_type {};

//----------------------------------------------------------------------
// sst::is_sign_magnitude
//----------------------------------------------------------------------

template<class T = int, class = void>
struct is_sign_magnitude : std::false_type {};

template<class T>
struct is_sign_magnitude<
    T,
    typename std::enable_if<is_signed_integer<T>()
                            && ~1 == type_min<int>() + 1>::type>
    : std::true_type {};

//----------------------------------------------------------------------
// sst::is_exact_width_integer
//----------------------------------------------------------------------

template<class, class = void>
struct is_exact_width_integer : std::false_type {};

template<class T>
struct is_exact_width_integer<
    T,
    typename std::enable_if<
        false
#ifdef INT8_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::int8_t>::value
#endif
#ifdef UINT8_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::uint8_t>::value
#endif
#ifdef INT16_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::int16_t>::value
#endif
#ifdef UINT16_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::uint16_t>::value
#endif
#ifdef INT32_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::int32_t>::value
#endif
#ifdef UINT32_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::uint32_t>::value
#endif
#ifdef INT64_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::int64_t>::value
#endif
#ifdef UINT64_MAX
        || std::is_same<typename std::remove_cv<T>::type,
                        std::uint64_t>::value
#endif
        >::type> : std::true_type {
};

//----------------------------------------------------------------------
// sst::is_bool
//----------------------------------------------------------------------
template<class, class = void>
struct is_bool : std::false_type {};

template<class T>
struct is_bool<T,
               typename std::enable_if<
                   std::is_same<typename std::remove_cv<T>::type,
                                bool>::value>::type> : std::true_type {
};

//----------------------------------------------------------------------
// sst::is_non_bool_integer
//----------------------------------------------------------------------

template<class, class = void>
struct is_non_bool_integer : std::false_type {};

template<class T>
struct is_non_bool_integer<
    T,
    typename std::enable_if<std::is_integral<T>()
                            && !sst::is_bool<T>()>::type>
    : std::true_type {};

//----------------------------------------------------------------------
// sst::is_little_endian
//----------------------------------------------------------------------

template<class, class = void>
struct is_little_endian : std::false_type {};

template<class T>
struct is_little_endian<
    T,
    typename std::enable_if<
        false
        || (sst::is_non_bool_integer<T>::value
            && (sizeof(T) == 1
#if PFSS_CPP20_OR_LATER
                || std::endian::native == std::endian::little
#endif
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
                || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#endif
                ))>::type> : std::true_type {
};

//----------------------------------------------------------------------
// sst::is_big_endian
//----------------------------------------------------------------------

template<class, class = void>
struct is_big_endian : std::false_type {};

template<class T>
struct is_big_endian<
    T,
    typename std::enable_if<
        false
        || (sst::is_non_bool_integer<T>::value
            && (sizeof(T) == 1
#if PFSS_CPP20_OR_LATER
                || std::endian::native == std::endian::big
#endif
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
                || __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#endif
                ))>::type> : std::true_type {
};

//----------------------------------------------------------------------
// sst::promote_unsigned
//----------------------------------------------------------------------

template<class T>
class promote_unsigned {
  PFSS_STATIC_ASSERT(std::is_integral<T>::value);
  using T1 = typename std::make_unsigned<decltype(+T())>::type;
  using T2 =
      typename std::conditional<std::is_const<T>::value,
                                typename std::add_const<T1>::type,
                                T1>::type;
  using T3 =
      typename std::conditional<std::is_volatile<T>::value,
                                typename std::add_volatile<T2>::type,
                                T2>::type;

public:
  using type = T3;
};

//----------------------------------------------------------------------
// sst::is_negative
//----------------------------------------------------------------------

template<class T,
         typename std::enable_if<sst::is_signed_integer<T>::value,
                                 int>::type = 0>
constexpr bool is_negative(T const x) noexcept {
  return x < 0;
}

template<class T,
         typename std::enable_if<!sst::is_signed_integer<T>::value,
                                 int>::type = 0>
constexpr bool is_negative(T) noexcept {
  PFSS_STATIC_ASSERT(sst::is_unsigned_integer<T>::value);
  return false;
}

//----------------------------------------------------------------------
// sst::to_unsigned
//----------------------------------------------------------------------

template<class T>
constexpr typename promote_unsigned<T>::type
to_unsigned(T const x) noexcept {
  return static_cast<typename promote_unsigned<T>::type>(x);
}

//----------------------------------------------------------------------
// sst::unsigned_lt
//----------------------------------------------------------------------

template<class A, class B>
constexpr bool unsigned_lt(A const a, B const b) noexcept {
  return to_unsigned(a) < to_unsigned(b);
}

//----------------------------------------------------------------------
// sst::unsigned_gt
//----------------------------------------------------------------------

template<class A, class B>
constexpr bool unsigned_gt(A const a, B const b) noexcept {
  return to_unsigned(a) > to_unsigned(b);
}

//----------------------------------------------------------------------
// sst::unsigned_le
//----------------------------------------------------------------------

template<class A, class B>
constexpr bool unsigned_le(A const a, B const b) noexcept {
  return to_unsigned(a) <= to_unsigned(b);
}

//----------------------------------------------------------------------
// sst::unsigned_ge
//----------------------------------------------------------------------

template<class A, class B>
constexpr bool unsigned_ge(A const a, B const b) noexcept {
  return to_unsigned(a) >= to_unsigned(b);
}

//----------------------------------------------------------------------
// sst::unsigned_eq
//----------------------------------------------------------------------

template<class A, class B>
constexpr bool unsigned_eq(A const a, B const b) noexcept {
  return to_unsigned(a) == to_unsigned(b);
}

//----------------------------------------------------------------------
// sst::unsigned_ne
//----------------------------------------------------------------------

template<class A, class B>
constexpr bool unsigned_ne(A const a, B const b) noexcept {
  return to_unsigned(a) != to_unsigned(b);
}

//----------------------------------------------------------------------
// sst::perfect_lt
//----------------------------------------------------------------------

template<class T, class U>
constexpr bool perfect_lt(T const a, U const b) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<T>());
  PFSS_STATIC_ASSERT(std::is_integral<U>());
  using Tp = typename sst::integer_promote<T>::type;
  using Up = typename sst::integer_promote<U>::type;
  using Ts = typename std::make_signed<Tp>::type;
  using Us = typename std::make_signed<Up>::type;
  return sst::is_negative(a) ?
             !sst::is_negative(b)
                 || static_cast<Ts>(a) < static_cast<Us>(b) :
             !sst::is_negative(b) && sst::unsigned_lt(a, b);
}

//----------------------------------------------------------------------
// sst::perfect_gt
//----------------------------------------------------------------------

template<class T, class U>
constexpr bool perfect_gt(T const a, U const b) noexcept {
  return sst::perfect_lt(b, a);
}

//----------------------------------------------------------------------
// sst::perfect_le
//----------------------------------------------------------------------

template<class T, class U>
constexpr bool perfect_le(T const a, U const b) noexcept {
  return !sst::perfect_gt(a, b);
}

//----------------------------------------------------------------------
// sst::perfect_ge
//----------------------------------------------------------------------

template<class T, class U>
constexpr bool perfect_ge(T const a, U const b) noexcept {
  return !sst::perfect_lt(a, b);
}

//----------------------------------------------------------------------
// sst::perfect_eq
//----------------------------------------------------------------------

template<class T, class U>
constexpr bool perfect_eq(T const a, U const b) noexcept {
  return sst::perfect_le(a, b) && sst::perfect_le(b, a);
}

//----------------------------------------------------------------------
// sst::perfect_ne
//----------------------------------------------------------------------

template<class T, class U>
constexpr bool perfect_ne(T const a, U const b) noexcept {
  return !sst::perfect_eq(a, b);
}

//----------------------------------------------------------------------
// sst::can_represent_all
//----------------------------------------------------------------------

namespace detail {

template<class T, class U, class = void>
struct can_represent_all_2 : std::false_type {};

template<class T, class U>
struct can_represent_all_2<
    T,
    U,
    typename std::enable_if<sst::perfect_le(sst::type_min<T>::value,
                                             sst::type_min<U>::value)
                            && sst::perfect_ge(
                                sst::type_max<T>::value,
                                sst::type_max<U>::value)>::type>
    : std::true_type {};

} // namespace detail

template<class T, class U, class = void>
struct can_represent_all : std::false_type {};

template<class T, class U>
struct can_represent_all<
    T,
    U,
    typename std::enable_if<std::is_integral<T>()
                            && std::is_integral<U>()>::type>
    : detail::can_represent_all_2<T, U> {};

//----------------------------------------------------------------------
// sst::get_bit
//----------------------------------------------------------------------

template<
    class X,
    class Q,
    class R,
    typename std::enable_if<std::is_integral<X>::value, int>::type = 0>
constexpr bool get_bit(X const x, Q const q, R const r) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<X>::value);
  PFSS_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_STATIC_ASSERT(std::is_integral<R>::value);
  PFSS_CONSTEXPR_ASSERT(!is_negative(q));
  PFSS_CONSTEXPR_ASSERT(!is_negative(r));
  using UX = typename promote_unsigned<X>::type;
#define PFSS_X_is_signed (std::is_signed<X>::value)
#define PFSS_X_width (std::numeric_limits<X>::digits + PFSS_X_is_signed)
#define PFSS_char_bit (std::numeric_limits<unsigned char>::digits)
  PFSS_CONSTEXPR_ASSERT(unsigned_le(r, PFSS_X_width - 1));
  PFSS_CONSTEXPR_ASSERT(unsigned_le(
      q,
      (PFSS_X_width - 1 - static_cast<int>(r)) / PFSS_char_bit));
#define PFSS_i \
  (static_cast<int>(q) * PFSS_char_bit + static_cast<int>(r))
#define PFSS_mask (static_cast<UX>(1) << PFSS_i)
  return PFSS_X_is_signed && PFSS_i == PFSS_X_width - 1 ?
             is_negative(x) :
             static_cast<bool>(static_cast<UX>(x) & PFSS_mask);
#undef PFSS_mask
#undef PFSS_i
#undef PFSS_char_bit
#undef PFSS_X_width
#undef PFSS_X_is_signed
}

template<
    class X,
    class R,
    typename std::enable_if<std::is_integral<X>::value, int>::type = 0>
constexpr bool get_bit(X const x, R const r) noexcept {
  return get_bit(x, 0, r);
}

template<class Q, class R>
bool get_bit(void const * const x, Q const q, R const r) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_STATIC_ASSERT(std::is_integral<R>::value);
  assert(x != nullptr);
  assert(!is_negative(q));
  assert(!is_negative(r));
  auto const rq = r / std::numeric_limits<unsigned char>::digits;
  auto const rr = r % std::numeric_limits<unsigned char>::digits;
  return static_cast<bool>(
      *(static_cast<unsigned char const *>(x) + q + rq)
      & static_cast<unsigned char>(1U << rr));
}

template<class R>
bool get_bit(void const * const x, R const r) noexcept {
  return get_bit(x, 0, r);
}

//----------------------------------------------------------------------
// sst::set_bit
//----------------------------------------------------------------------

template<
    class X,
    class Q,
    class R,
    typename std::enable_if<std::is_integral<X>::value, int>::type = 0>
constexpr X
set_bit(X const x, Q const q, R const r, bool const b) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<X>::value);
  PFSS_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_STATIC_ASSERT(std::is_integral<R>::value);
  PFSS_CONSTEXPR_ASSERT(!is_negative(q));
  PFSS_CONSTEXPR_ASSERT(!is_negative(r));
  using UX = typename promote_unsigned<X>::type;
#define PFSS_X_max (std::numeric_limits<X>::max())
#define PFSS_X_is_signed (std::is_signed<X>::value)
#define PFSS_X_width (std::numeric_limits<X>::digits + PFSS_X_is_signed)
#define PFSS_char_bit (std::numeric_limits<unsigned char>::digits)
  PFSS_CONSTEXPR_ASSERT(unsigned_le(r, PFSS_X_width - 1));
  PFSS_CONSTEXPR_ASSERT(unsigned_le(
      q,
      (PFSS_X_width - 1 - static_cast<int>(r)) / PFSS_char_bit));
#define PFSS_i \
  (static_cast<int>(q) * PFSS_char_bit + static_cast<int>(r))
#define PFSS_mask (static_cast<UX>(1) << PFSS_i)
  return PFSS_X_is_signed && PFSS_i == PFSS_X_width - 1 ?
             (b == is_negative(x) ?
                  x :
                  b ? x - PFSS_X_max - static_cast<X>(1) :
                      x + PFSS_X_max + static_cast<X>(1)) :
             static_cast<X>((b ? static_cast<UX>(x) | PFSS_mask :
                                 static_cast<UX>(x) & ~PFSS_mask)
                            & static_cast<UX>(PFSS_X_max))
                 - (is_negative(x) ? PFSS_X_max : static_cast<X>(0))
                 - (is_negative(x) ? static_cast<X>(1) :
                                     static_cast<X>(0));
#undef PFSS_mask
#undef PFSS_i
#undef PFSS_char_bit
#undef PFSS_X_width
#undef PFSS_X_is_signed
#undef PFSS_X_max
}

template<
    class X,
    class R,
    typename std::enable_if<std::is_integral<X>::value, int>::type = 0>
constexpr X set_bit(X const x, R const r, bool const b) noexcept {
  return set_bit(x, 0, r, b);
}

template<class Q, class R>
void *
set_bit(void * const x, Q const q, R const r, bool const b) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_STATIC_ASSERT(std::is_integral<R>::value);
  assert(x != nullptr);
  assert(!is_negative(q));
  assert(!is_negative(r));
  auto const rq = r / std::numeric_limits<unsigned char>::digits;
  auto const rr = r % std::numeric_limits<unsigned char>::digits;
  if (b) {
    *(static_cast<unsigned char *>(x) + q + rq) |=
        static_cast<unsigned char>(1U << rr);
  } else {
    *(static_cast<unsigned char *>(x) + q + rq) &=
        static_cast<unsigned char>(~(1U << rr));
  }
  return x;
}

template<class R>
void * set_bit(void * const x, R const r, bool const b) noexcept {
  return set_bit(x, 0, r, b);
}

//----------------------------------------------------------------------
// sst::from_bits
//----------------------------------------------------------------------

template<class Y,
         bool SignExtend = std::is_signed<Y>::value,
         class Q,
         class R,
         class N>
Y from_bits(void const * const x,
            Q const q,
            R const r,
            N const n) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<Y>::value);
  PFSS_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_STATIC_ASSERT(std::is_integral<R>::value);
  PFSS_STATIC_ASSERT(std::is_integral<N>::value);
  assert(x != nullptr);
  assert(!is_negative(q));
  assert(!is_negative(r));
  assert(n > 0);
  using sz = typename promote_unsigned<std::size_t>::type;
  using UR = typename promote_unsigned<R>::type;
  Y y = 0;
  constexpr bool Y_is_signed = std::is_signed<Y>::value;
  constexpr int Y_width = std::numeric_limits<Y>::digits + Y_is_signed;
  constexpr sz char_bit = std::numeric_limits<unsigned char>::digits;
  constexpr sz size_max = std::numeric_limits<std::size_t>::max();
  sz i = static_cast<sz>(q)
         + static_cast<sz>(static_cast<UR>(r) / char_bit);
  sz j = static_cast<sz>(static_cast<UR>(r) % char_bit);
  int k = 0;
  bool b = false;
  for (; k != Y_width && unsigned_ne(k, n); ++k) {
    b = get_bit(x, i, j);
    y = set_bit(y, k, b);
    if (j == size_max) {
      i += j / char_bit;
      j %= char_bit;
    }
    ++j;
  }
  if (SignExtend && b) {
    for (; k != Y_width; ++k) {
      y = set_bit(y, k, b);
    }
  }
  return y;
}

template<class Y,
         bool SignExtend = std::is_signed<Y>::value,
         class R,
         class N>
Y from_bits(void const * const x, R const r, N const n) noexcept {
  return from_bits<Y, SignExtend>(x, 0, r, n);
}

//----------------------------------------------------------------------
// sst::to_bits
//----------------------------------------------------------------------

template<class Y,
         bool SignExtend = std::is_signed<Y>::value,
         class Q,
         class R,
         class N>
void * to_bits(void * const x,
               Q const q,
               R const r,
               Y const y,
               N const n) noexcept {
  PFSS_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_STATIC_ASSERT(std::is_integral<R>::value);
  PFSS_STATIC_ASSERT(std::is_integral<Y>::value);
  PFSS_STATIC_ASSERT(std::is_integral<N>::value);
  assert(x != nullptr);
  assert(!is_negative(q));
  assert(!is_negative(r));
  assert(n > 0);
  using sz = typename promote_unsigned<std::size_t>::type;
  using UR = typename promote_unsigned<R>::type;
  constexpr bool Y_is_signed = std::is_signed<Y>::value;
  constexpr int Y_width = std::numeric_limits<Y>::digits + Y_is_signed;
  constexpr sz char_bit = std::numeric_limits<unsigned char>::digits;
  constexpr sz size_max = std::numeric_limits<std::size_t>::max();
  sz i = static_cast<sz>(q)
         + static_cast<sz>(static_cast<UR>(r) / char_bit);
  sz j = static_cast<sz>(static_cast<UR>(r) % char_bit);
  int k = 0;
  bool b = false;
  for (; k != Y_width && unsigned_ne(k, n); ++k) {
    b = get_bit(y, k);
    set_bit(x, i, j, b);
    if (j == size_max) {
      i += j / char_bit;
      j %= char_bit;
    }
    ++j;
  }
  for (; unsigned_ne(k, n); ++k) {
    set_bit(x, i, j, SignExtend ? b : 0);
    if (j == size_max) {
      i += j / char_bit;
      j %= char_bit;
    }
    ++j;
  }
  return x;
}

template<class Y,
         bool SignExtend = std::is_signed<Y>::value,
         class R,
         class N>
void *
to_bits(void * const x, R const r, Y const y, N const n) noexcept {
  return to_bits<Y, SignExtend>(x, 0, r, y, n);
}

//----------------------------------------------------------------------
// sst::max
//----------------------------------------------------------------------

template<class T>
constexpr T const & max(T const & a, T const & b) {
  return a < b ? b : a;
}

//----------------------------------------------------------------------
// sst::min
//----------------------------------------------------------------------

template<class T>
constexpr T const & min(T const & a, T const & b) {
  return a < b ? a : b;
}

//----------------------------------------------------------------------
// sst::checked_error
//----------------------------------------------------------------------

struct checked_error : std::range_error {
  using std::range_error::range_error;
};

//----------------------------------------------------------------------
// sst::checked
//----------------------------------------------------------------------

template<class IntType, class = void>
class checked;

template<class IntType>
class checked<
    IntType,
    typename std::enable_if<std::is_integral<IntType>::value>::type>
    final {

  static PFSS_CPP14_CONSTEXPR void check(bool const x) {
    if (!x) {
      throw sst::checked_error("value out of range");
    }
  }

  IntType v_ = 0;

public:
  using value_type = IntType;

  // Value retrieval.
  constexpr IntType v() const noexcept {
    return v_;
  }
  constexpr IntType value() const noexcept {
    return v_;
  }
  constexpr IntType operator()() const noexcept {
    return v_;
  }

  //--------------------------------------------------------------------

  // The usual six.
  constexpr checked() noexcept = default;
  constexpr checked(checked const &) noexcept = default;
  PFSS_CPP14_CONSTEXPR checked &
  operator=(checked const &) noexcept = default;
  constexpr checked(checked &&) noexcept = default;
  PFSS_CPP14_CONSTEXPR checked &
  operator=(checked &&) noexcept = default;
  ~checked() noexcept = default;

  // Implicit construction from integer types subsumed by IntType.
  template<class IntType2,
           typename std::enable_if<
               std::is_integral<IntType2>::value
                   && sst::can_represent_all<IntType, IntType2>::value,
               int>::type = 0>
  constexpr checked(IntType2 const x) noexcept
      : v_(static_cast<IntType>(x)) {
  }

  // Implicit construction from integer types not subsumed by IntType.
  template<
      class IntType2,
      typename std::enable_if<
          std::is_integral<IntType2>::value
              && !sst::can_represent_all<IntType, IntType2>::value,
          int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked(IntType2 const x)
      : v_(static_cast<IntType>(x)) {
    check(sst::perfect_ge(x, sst::type_min<IntType>::value));
    check(sst::perfect_le(x, sst::type_max<IntType>::value));
  }

  // Implicit construction from checked types subsumed by IntType.
  template<class IntType2,
           typename std::enable_if<
               std::is_integral<IntType2>::value
                   && sst::can_represent_all<IntType, IntType2>::value,
               int>::type = 0>
  constexpr checked(checked<IntType2> const x) noexcept
      : checked(x.v()) {
  }

  // Implicit construction from checked types not subsumed by IntType.
  template<
      class IntType2,
      typename std::enable_if<
          std::is_integral<IntType2>::value
              && !sst::can_represent_all<IntType, IntType2>::value,
          int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked(checked<IntType2> const x)
      : checked(x.v()) {
  }

  //--------------------------------------------------------------------
  // Conversions
  //--------------------------------------------------------------------

  template<class U,
           typename std::enable_if<
               std::is_integral<U>()
                   && sst::can_represent_all<U, IntType>(),
               int>::type = 0>
  explicit constexpr operator U() const noexcept {
    return static_cast<U>(v());
  }

  template<class U,
           typename std::enable_if<
               std::is_integral<U>()
                   && !sst::can_represent_all<U, IntType>(),
               int>::type = 0>
  explicit PFSS_CPP14_CONSTEXPR operator U() const {
    return checked<U>(v()).v();
  }

  //--------------------------------------------------------------------
  // Non-comparison core
  //--------------------------------------------------------------------

  // a++
  PFSS_CPP14_CONSTEXPR checked operator++(int) {
    checked const x = *this;
    ++*this;
    return x;
  }

  // a--
  PFSS_CPP14_CONSTEXPR checked operator--(int) {
    checked const x = *this;
    --*this;
    return x;
  }

  // ++a
  PFSS_CPP14_CONSTEXPR checked operator++() {
    return *this += static_cast<IntType>(1);
  }

  // --a
  PFSS_CPP14_CONSTEXPR checked operator--() {
    return *this -= static_cast<IntType>(1);
  }

  // +a
  constexpr checked<decltype(+IntType())> operator+() const noexcept {
    return v_;
  }

  // -a
  PFSS_CPP14_CONSTEXPR checked<decltype(-IntType())> operator-() const {
    using R = decltype(-IntType());
    R const a = checked<R>(*this).v();
    if (std::is_signed<R>()) {
      check(a >= -type_max<R>());
    } else {
      check(a == 0);
    }
    return -a;
  }

  // ~a
  PFSS_CPP14_CONSTEXPR checked<decltype(~IntType())> operator~() const
      noexcept(is_twos_complement<decltype(~IntType())>()) {
    using R = decltype(~IntType());
    R const a = checked<R>(*this).v();
    if (is_ones_complement<R>()) {
      check(a != 0);
    } else if (is_sign_magnitude<R>()) {
      check(a != type_max<R>());
    }
    return ~a;
  }

  // !a
  constexpr checked<bool> operator!() const noexcept {
    return !v();
  }

  // a * b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() * U())>
  operator*(checked<U> const x) const {
    using R = decltype(IntType() * U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    if (a > 0 && b > 0) {
      check(a <= type_max<R>() / b);
    } else if (a > 0 && is_negative(b)) {
      check(a <= type_min<R>() / b);
    } else if (is_negative(a) && b > 0) {
      check(a >= type_min<R>() / b);
    } else if (is_negative(a) && is_negative(b)) {
      check(a >= type_max<R>() / b);
    }
    return a * b;
  }

  // a / b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() / U())>
  operator/(checked<U> const x) const {
    using R = decltype(IntType() / U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    check(b != 0);
    return a / b;
  }

  // a % b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() % U())>
  operator%(checked<U> const x) const {
    using R = decltype(IntType() % U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    check(b != 0);
    return a % b;
  }

  // a + b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() + U())>
  operator+(checked<U> const x) const {
    using R = decltype(IntType() + U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    if (a > 0 && b > 0) {
      check(a <= type_max<R>() - b);
    } else if (is_negative(a) && is_negative(b)) {
      check(a >= type_min<R>() - b);
    }
    return a + b;
  }

  // a - b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() - U())>
  operator-(checked<U> const x) const {
    using R = decltype(IntType() - U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    if (std::is_unsigned<R>() && b > 0) {
      check(a >= b);
    } else if (!is_negative(a) && is_negative(b)) {
      check(a <= type_max<R>() + b);
    } else if (is_negative(a) && b > 0) {
      check(a >= type_min<R>() + b);
    }
    return a - b;
  }

  // a << b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() << U())>
  operator<<(checked<U> const x) const {
    using R = decltype(IntType() << U());
    R const a = checked<R>(*this).v();
    U const b = x.v();
    check(!is_negative(a));
    check(!is_negative(b));
    check(unsigned_lt(b, value_bits<R>::value));
    check(a <= (type_max<R>() >> b));
    return a << b;
  }

  // a >> b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() >> U())>
  operator>>(checked<U> const x) const {
    using R = decltype(IntType() >> U());
    R const a = checked<R>(*this).v();
    U const b = x.v();
    check(!is_negative(a));
    check(!is_negative(b));
    check(unsigned_lt(b, value_bits<R>::value));
    return a >> b;
  }

  // a & b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() & U())>
  operator&(checked<U> const x) const
      noexcept(is_twos_complement<decltype(IntType() & U())>()
               || is_ones_complement<decltype(IntType() & U())>()) {
    using R = decltype(IntType() & U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    if (is_sign_magnitude<R>()) {
      if (is_negative(a) && is_negative(b)) {
        check((-a & -b) != 0);
      }
    }
    return a & b;
  }

  // a ^ b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() ^ U())>
  operator^(checked<U> const x) const
      noexcept(is_twos_complement<decltype(IntType() ^ U())>()) {
    using R = decltype(IntType() ^ U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    if (is_negative(a) != is_negative(b)) {
      R const aa = is_negative(a) ? a : b;
      R const bb = is_negative(a) ? b : a;
      if (is_ones_complement<R>()) {
        check(-aa != bb);
      } else if (is_sign_magnitude<R>()) {
        check((-aa ^ bb) != 0);
      }
    }
    return a ^ b;
  }

  // a | b
  template<
      class U,
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>
  PFSS_CPP14_CONSTEXPR checked<decltype(IntType() | U())>
  operator|(checked<U> const x) const
      noexcept(is_twos_complement<decltype(IntType() | U())>()
               || is_sign_magnitude<decltype(IntType() | U())>()) {
    using R = decltype(IntType() | U());
    R const a = checked<R>(*this).v();
    R const b = checked<R>(x).v();
    if (is_ones_complement<R>()) {
      if (is_negative(a) || is_negative(b)) {
        R const aa = is_negative(a) ? type_max<R>() + a : a;
        R const bb = is_negative(b) ? type_max<R>() + b : b;
        check((aa | bb) != type_max<R>());
      }
    }
    return a | b;
  }

  //--------------------------------------------------------------------
  // Non-comparison sugar
  //--------------------------------------------------------------------

#define PFSS_DEFINE_FUNCTIONS(op) \
\
  template< \
      class U, \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0> \
  PFSS_CPP14_CONSTEXPR auto operator op(U const b) \
      const->decltype(*this op checked<U>(b)) { \
    return *this op checked<U>(b); \
  } \
\
  template<class U, \
           typename std::enable_if<+std::is_integral<IntType>(), \
                                   int>::type = 0> \
  friend PFSS_CPP14_CONSTEXPR auto operator op(U const a, \
                                               checked const b) \
      ->decltype(checked<U>(a) op b) { \
    return checked<U>(a) op b; \
  } \
\
  template< \
      class U, \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0> \
  PFSS_CPP14_CONSTEXPR checked & operator op##=(checked<U> const b) { \
    return *this = *this op b; \
  } \
\
  template< \
      class U, \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0> \
  PFSS_CPP14_CONSTEXPR checked & operator op##=(U const b) { \
    return *this op## = checked<U>(b); \
  }

  PFSS_DEFINE_FUNCTIONS(*)
  PFSS_DEFINE_FUNCTIONS(/)
  PFSS_DEFINE_FUNCTIONS(%)
  PFSS_DEFINE_FUNCTIONS(+)
  PFSS_DEFINE_FUNCTIONS(-)
  PFSS_DEFINE_FUNCTIONS(<<)
  PFSS_DEFINE_FUNCTIONS(>>)
  PFSS_DEFINE_FUNCTIONS(&)
  PFSS_DEFINE_FUNCTIONS(^)
  PFSS_DEFINE_FUNCTIONS(|)

#undef PFSS_DEFINE_FUNCTIONS

  //--------------------------------------------------------------------
  // Comparison
  //--------------------------------------------------------------------

#define PFSS_DEFINE_FUNCTIONS(op) \
\
  template< \
      class U, \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0> \
  PFSS_CPP14_CONSTEXPR checked<bool> operator op(checked<U> const b) \
      const { \
    using R = decltype(IntType() + U()); \
    return checked<R>(*this).v() op checked<R>(b).v(); \
  } \
\
  template< \
      class U, \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0> \
  PFSS_CPP14_CONSTEXPR checked<bool> operator op(U const b) const { \
    return *this op checked<U>(b); \
  } \
\
  template< \
      class U, \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0> \
  friend PFSS_CPP14_CONSTEXPR checked<bool> operator op( \
      U const a, \
      checked const b) { \
    return checked<U>(a) op b; \
  }

  PFSS_DEFINE_FUNCTIONS(<)
  PFSS_DEFINE_FUNCTIONS(>)
  PFSS_DEFINE_FUNCTIONS(<=)
  PFSS_DEFINE_FUNCTIONS(>=)
  PFSS_DEFINE_FUNCTIONS(==)
  PFSS_DEFINE_FUNCTIONS(!=)

#undef PFSS_DEFINE_FUNCTIONS

  //--------------------------------------------------------------------
};

//----------------------------------------------------------------------
// sst::checked_cast
//----------------------------------------------------------------------

template<class T, class U>
PFSS_CPP14_CONSTEXPR T checked_cast(U const x) {
  return checked<T>(x).v();
}

//----------------------------------------------------------------------
// sst::boxed
//----------------------------------------------------------------------

template<class T, class>
class boxed {
  T value_;

public:
  using value_type = T;

  // Construction is forwarded to T.
  template<class... Args>
  explicit boxed(Args &&... args) noexcept(
      noexcept(T(std::forward<Args>(args)...)))
      : value_(std::forward<Args>(args)...) {
  }

  // Copy construction and assignment are intentionally implicit.
  // Move construction and assignment are intentionally implicit.
  // Destruction is intentionally implicit.

  // Value retrieval.
  explicit PFSS_CPP14_CONSTEXPR operator T &() noexcept {
    return value_;
  }
  explicit constexpr operator T const &() const noexcept {
    return value_;
  }
  PFSS_CPP14_CONSTEXPR T & value() noexcept {
    return value_;
  }
  constexpr T const & value() const noexcept {
    return value_;
  }
};

//----------------------------------------------------------------------
// sst::bit_scan_reverse
//----------------------------------------------------------------------

namespace _ {
namespace bit_scan_reverse {

template<class, class = void>
struct can_use_x86_intrinsic : std::false_type {};

template<class T>
struct can_use_x86_intrinsic<
    T,
    typename std::enable_if<PFSS_HAVE_X86 && std::is_integral<T>::value
                            && (sst::width_bits<T>::value
                                <= 32)>::type> : std::true_type {};

template<class T>
int fallback(T const & x) {
  assert(x != static_cast<T>(0));
  if (sst::is_negative(x)) {
    return sst::width_bits<T>::value - 1;
  }
  T m = sst::type_max<T>::value >> 1;
  int i = sst::value_bits<T>::value - 1;
  while ((x & m) == x) {
    m >>= 1;
    --i;
  }
  return i;
}

} // namespace bit_scan_reverse
} // namespace _

#if PFSS_HAVE_X86

template<class T,
         typename std::enable_if<
             sst::_::bit_scan_reverse::can_use_x86_intrinsic<T>::value
                 && (sst::width_bits<T>::value == 32),
             int>::type = 0>
int bit_scan_reverse(T const x) noexcept {
  assert(x != static_cast<T>(0));
  return _bit_scan_reverse(static_cast<int>(x));
}

template<class T,
         typename std::enable_if<
             sst::_::bit_scan_reverse::can_use_x86_intrinsic<T>::value
                 && (sst::width_bits<T>::value < 32),
             int>::type = 0>
int bit_scan_reverse(T const x) noexcept {
  assert(x != static_cast<T>(0));
  if (sst::is_negative(x)) {
    return sst::width_bits<T>::value - 1;
  }
  return _bit_scan_reverse(static_cast<int>(x));
}

#endif // PFSS_HAVE_X86

template<
    class T,
    typename std::enable_if<
        !sst::_::bit_scan_reverse::can_use_x86_intrinsic<T>::value,
        int>::type = 0>
int bit_scan_reverse(T const & x) {
  return sst::_::bit_scan_reverse::fallback(x);
}

//----------------------------------------------------------------------
// sst::to_string
//----------------------------------------------------------------------

template<class, class = void>
struct to_string_functor;

template<class T>
struct to_string_functor<
    T,
    typename std::enable_if<std::is_integral<T>::value>::type> {
  template<class OutputIt>
  OutputIt operator()(T const & x, OutputIt out) {
    T d = 1;
    if (x < 0) {
      while (d <= x / -10) {
        d *= 10;
      }
      *out++ = '-';
      for (; d != 0; d /= 10) {
        *out++ = static_cast<char>(
            '0' + static_cast<char>(-static_cast<int>((x / d) % 10)));
      }
    } else {
      while (d <= x / 10) {
        d *= 10;
      }
      for (; d != 0; d /= 10) {
        *out++ =
            static_cast<char>('0' + static_cast<char>((x / d) % 10));
      }
    }
    return out;
  }
};

template<class T, class OutputIt>
OutputIt to_string(T const & x, OutputIt const out) {
  return to_string_functor<T>()(x, out);
}

template<class T>
std::string to_string(T const & x) {
  std::string s;
  to_string(x, std::back_inserter(s));
  return s;
}

//----------------------------------------------------------------------
// sst::unix_time
//----------------------------------------------------------------------

template<class Duration = std::chrono::seconds>
typename Duration::rep unix_time() {
  return std::chrono::duration_cast<Duration>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

//----------------------------------------------------------------------
// sst::unix_time_ns
//----------------------------------------------------------------------

inline std::chrono::nanoseconds::rep unix_time_ns() {
  return unix_time<std::chrono::nanoseconds>();
}

//----------------------------------------------------------------------
// sst::mono_time
//----------------------------------------------------------------------

PFSS_PUBLIC_CPP_OBJECT
std::once_flag mono_time_flag;

PFSS_PUBLIC_CPP_OBJECT
decltype(std::chrono::steady_clock::now()) mono_time_t0;

template<class Duration>
typename Duration::rep mono_time() {
  std::call_once(mono_time_flag, [] {
    mono_time_t0 = std::chrono::steady_clock::now();
  });
  return std::chrono::duration_cast<Duration>(
             std::chrono::steady_clock::now() - mono_time_t0)
      .count();
}

//----------------------------------------------------------------------
// sst::mono_time_ns
//----------------------------------------------------------------------

inline std::chrono::nanoseconds::rep mono_time_ns() {
  return mono_time<std::chrono::nanoseconds>();
}

//----------------------------------------------------------------------

} // namespace sst

// DEPRECATED: we'll eventually move to using SST
namespace pfss {
using namespace sst;
}

#endif // PFSS_BASE_HPP
