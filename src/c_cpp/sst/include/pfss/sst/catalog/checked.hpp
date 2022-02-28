//
// Copyright (C) 2012-2022 Stealth Software Technologies, Inc.
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program. If not, see
// <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#ifndef PFSS_SST_CATALOG_CHECKED_HPP
#define PFSS_SST_CATALOG_CHECKED_HPP

#include <limits>
#include <stdexcept>
#include <type_traits>

#include <pfss/sst/catalog/can_represent_all.hpp>
#include <pfss/sst/catalog/checked_overflow.hpp>
#include <pfss/sst/catalog/is_ones_complement.hpp>
#include <pfss/sst/catalog/is_sign_magnitude.hpp>
#include <pfss/sst/catalog/is_twos_complement.hpp>
#include <pfss/sst/catalog/perfect_ge.hpp>
#include <pfss/sst/catalog/perfect_le.hpp>
#include <pfss/sst/catalog/type_max.hpp>
#include <pfss/sst/catalog/type_min.hpp>
#include <pfss/sst/catalog/value_bits.hpp>
#include <pfss/sst/integer.h>
#include <pfss/sst/language.h>
#include <pfss/sst/limits.h>

namespace pfss { namespace sst {

template<class IntType, class = void>
class checked;

template<class IntType>
class checked<
    IntType,
    typename std::enable_if<std::is_integral<IntType>::value>::type>
    final {

  static PFSS_SST_CPP14_CONSTEXPR void check(bool const x) {
    if (!x) {
      throw ::pfss::sst::checked_overflow("::pfss::sst::checked: overflow");
    }
  }

  IntType value_ = 0;

public:
  using value_type = IntType;

  // Value retrieval.
  constexpr IntType v() const noexcept {
    return value_;
  }
  constexpr IntType value() const noexcept {
    return value_;
  }
  constexpr IntType operator()() const noexcept {
    return value_;
  }

  //--------------------------------------------------------------------

  // The usual six.
  constexpr checked() noexcept = default;
  constexpr checked(checked const &) noexcept = default;
  PFSS_SST_CPP14_CONSTEXPR checked &
  operator=(checked const &) noexcept = default;
  constexpr checked(checked &&) noexcept = default;
  PFSS_SST_CPP14_CONSTEXPR checked &
  operator=(checked &&) noexcept = default;
  ~checked() noexcept = default;

  // Implicit construction from integer types subsumed by IntType.
  template<class IntType2,
           typename std::enable_if<
               std::is_integral<IntType2>::value
                   && ::pfss::sst::can_represent_all<IntType, IntType2>::value,
               int>::type = 0>
  constexpr checked(IntType2 const x) noexcept
      : value_(static_cast<IntType>(x)) {
  }

  // Implicit construction from integer types not subsumed by IntType.
  template<class IntType2,
           typename std::enable_if<
               std::is_integral<IntType2>::value
                   && !::pfss::sst::can_represent_all<IntType, IntType2>::value,
               int>::type = 0>
  PFSS_SST_CPP14_CONSTEXPR checked(IntType2 const x)
      : value_(static_cast<IntType>(x)) {
    check(::pfss::sst::perfect_ge(x, ::pfss::sst::type_min<IntType>::value));
    check(::pfss::sst::perfect_le(x, ::pfss::sst::type_max<IntType>::value));
  }

  // Implicit construction from checked types subsumed by IntType.
  template<class IntType2,
           typename std::enable_if<
               std::is_integral<IntType2>::value
                   && ::pfss::sst::can_represent_all<IntType, IntType2>::value,
               int>::type = 0>
  constexpr checked(checked<IntType2> const x) noexcept
      : checked(x.v()) {
  }

  // Implicit construction from checked types not subsumed by IntType.
  template<class IntType2,
           typename std::enable_if<
               std::is_integral<IntType2>::value
                   && !::pfss::sst::can_represent_all<IntType, IntType2>::value,
               int>::type = 0>
  PFSS_SST_CPP14_CONSTEXPR checked(checked<IntType2> const x)
      : checked(x.v()) {
  }

  //--------------------------------------------------------------------
  // Conversions
  //--------------------------------------------------------------------

  template<
      class U,
      typename std::enable_if<
          std::is_integral<U>() && ::pfss::sst::can_represent_all<U, IntType>(),
          int>::type = 0>
  explicit constexpr operator U() const noexcept {
    return static_cast<U>(v());
  }

  template<class U,
           typename std::enable_if<
               std::is_integral<U>()
                   && !::pfss::sst::can_represent_all<U, IntType>(),
               int>::type = 0>
  explicit PFSS_SST_CPP14_CONSTEXPR operator U() const {
    return checked<U>(v()).v();
  }

  //--------------------------------------------------------------------
  // Non-comparison core
  //--------------------------------------------------------------------

  // a++
  PFSS_SST_CPP14_CONSTEXPR checked operator++(int) {
    checked const x = *this;
    ++*this;
    return x;
  }

  // a--
  PFSS_SST_CPP14_CONSTEXPR checked operator--(int) {
    checked const x = *this;
    --*this;
    return x;
  }

  // ++a
  PFSS_SST_CPP14_CONSTEXPR checked operator++() {
    return *this += static_cast<IntType>(1);
  }

  // --a
  PFSS_SST_CPP14_CONSTEXPR checked operator--() {
    return *this -= static_cast<IntType>(1);
  }

  // +a
  constexpr checked<decltype(+IntType())> operator+() const noexcept {
    return value_;
  }

  // -a
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(-IntType())> operator-() const {
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(~IntType())> operator~() const
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() * U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() / U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() % U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() + U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() - U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() << U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() >> U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() & U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() ^ U())>
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
  PFSS_SST_CPP14_CONSTEXPR checked<decltype(IntType() | U())>
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

#define PFSS_SST_DEFINE_FUNCTIONS(op)                                       \
                                                                       \
  template<                                                            \
      class U,                                                         \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>  \
  PFSS_SST_CPP14_CONSTEXPR auto operator op(U const b)                      \
      const->decltype(*this op checked<U>(b)) {                        \
    return *this op checked<U>(b);                                     \
  }                                                                    \
                                                                       \
  template<class U,                                                    \
           typename std::enable_if<+std::is_integral<IntType>(),       \
                                   int>::type = 0>                     \
  friend PFSS_SST_CPP14_CONSTEXPR auto operator op(U const a,               \
                                              checked const b)         \
      ->decltype(checked<U>(a) op b) {                                 \
    return checked<U>(a) op b;                                         \
  }                                                                    \
                                                                       \
  template<                                                            \
      class U,                                                         \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>  \
  PFSS_SST_CPP14_CONSTEXPR checked & operator op##=(checked<U> const b) {   \
    return *this = *this op b;                                         \
  }                                                                    \
                                                                       \
  template<                                                            \
      class U,                                                         \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>  \
  PFSS_SST_CPP14_CONSTEXPR checked & operator op##=(U const b) {            \
    return *this op## = checked<U>(b);                                 \
  }

  PFSS_SST_DEFINE_FUNCTIONS(*)
  PFSS_SST_DEFINE_FUNCTIONS(/)
  PFSS_SST_DEFINE_FUNCTIONS(%)
  PFSS_SST_DEFINE_FUNCTIONS(+)
  PFSS_SST_DEFINE_FUNCTIONS(-)
  PFSS_SST_DEFINE_FUNCTIONS(<<)
  PFSS_SST_DEFINE_FUNCTIONS(>>)
  PFSS_SST_DEFINE_FUNCTIONS(&)
  PFSS_SST_DEFINE_FUNCTIONS(^)
  PFSS_SST_DEFINE_FUNCTIONS(|)

#undef PFSS_SST_DEFINE_FUNCTIONS

  //--------------------------------------------------------------------
  // Comparison
  //--------------------------------------------------------------------

#define PFSS_SST_DEFINE_FUNCTIONS(op)                                       \
                                                                       \
  template<                                                            \
      class U,                                                         \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>  \
  PFSS_SST_CPP14_CONSTEXPR checked<bool> operator op(checked<U> const b)    \
      const {                                                          \
    using R = decltype(IntType() + U());                               \
    return checked<R>(*this).v() op checked<R>(b).v();                 \
  }                                                                    \
                                                                       \
  template<                                                            \
      class U,                                                         \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>  \
  PFSS_SST_CPP14_CONSTEXPR checked<bool> operator op(U const b) const {     \
    return *this op checked<U>(b);                                     \
  }                                                                    \
                                                                       \
  template<                                                            \
      class U,                                                         \
      typename std::enable_if<+std::is_integral<U>(), int>::type = 0>  \
  friend PFSS_SST_CPP14_CONSTEXPR checked<bool> operator op(                \
      U const a,                                                       \
      checked const b) {                                               \
    return checked<U>(a) op b;                                         \
  }

  PFSS_SST_DEFINE_FUNCTIONS(<)
  PFSS_SST_DEFINE_FUNCTIONS(>)
  PFSS_SST_DEFINE_FUNCTIONS(<=)
  PFSS_SST_DEFINE_FUNCTIONS(>=)
  PFSS_SST_DEFINE_FUNCTIONS(==)
  PFSS_SST_DEFINE_FUNCTIONS(!=)

#undef PFSS_SST_DEFINE_FUNCTIONS

  //--------------------------------------------------------------------
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_CHECKED_HPP
