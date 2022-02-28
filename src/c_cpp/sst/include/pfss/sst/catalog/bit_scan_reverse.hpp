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

#ifndef PFSS_SST_CATALOG_BIT_SCAN_REVERSE_HPP
#define PFSS_SST_CATALOG_BIT_SCAN_REVERSE_HPP

#include <cassert>
#include <type_traits>

#include <pfss/sst/catalog/is_negative.hpp>
#include <pfss/sst/catalog/type_max.hpp>
#include <pfss/sst/catalog/value_bits.hpp>
#include <pfss/sst/catalog/width_bits.hpp>
#include <pfss/sst/config.h>

#if PFSS_SST_WITH_X86
#include <x86intrin.h>
#endif

namespace pfss { namespace sst {

namespace _ {
namespace bit_scan_reverse {

template<class, class = void>
struct can_use_x86_intrinsic : std::false_type {};

template<class T>
struct can_use_x86_intrinsic<
    T,
    typename std::enable_if<PFSS_SST_WITH_X86 && std::is_integral<T>::value
                            && (::pfss::sst::width_bits<T>::value <= 32)>::type>
    : std::true_type {};

template<class T>
int fallback(T const & x) {
  assert(x != static_cast<T>(0));
  if (::pfss::sst::is_negative(x)) {
    return ::pfss::sst::width_bits<T>::value - 1;
  }
  T m = ::pfss::sst::type_max<T>::value >> 1;
  int i = ::pfss::sst::value_bits<T>::value - 1;
  while ((x & m) == x) {
    m >>= 1;
    --i;
  }
  return i;
}

} // namespace bit_scan_reverse
} // namespace _

#if PFSS_SST_WITH_X86

template<class T,
         typename std::enable_if<
             ::pfss::sst::_::bit_scan_reverse::can_use_x86_intrinsic<T>::value
                 && (::pfss::sst::width_bits<T>::value == 32),
             int>::type = 0>
int bit_scan_reverse(T const x) noexcept {
  assert(x != static_cast<T>(0));
  return _bit_scan_reverse(static_cast<int>(x));
}

template<class T,
         typename std::enable_if<
             ::pfss::sst::_::bit_scan_reverse::can_use_x86_intrinsic<T>::value
                 && (::pfss::sst::width_bits<T>::value < 32),
             int>::type = 0>
int bit_scan_reverse(T const x) noexcept {
  assert(x != static_cast<T>(0));
  if (::pfss::sst::is_negative(x)) {
    return ::pfss::sst::width_bits<T>::value - 1;
  }
  return _bit_scan_reverse(static_cast<int>(x));
}

#endif // PFSS_SST_WITH_X86

template<class T,
         typename std::enable_if<
             !::pfss::sst::_::bit_scan_reverse::can_use_x86_intrinsic<T>::value,
             int>::type = 0>
int bit_scan_reverse(T const & x) {
  return ::pfss::sst::_::bit_scan_reverse::fallback(x);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_BIT_SCAN_REVERSE_HPP
