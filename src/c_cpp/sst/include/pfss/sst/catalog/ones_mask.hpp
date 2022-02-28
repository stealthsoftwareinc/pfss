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

#ifndef PFSS_SST_CATALOG_ONES_MASK_HPP
#define PFSS_SST_CATALOG_ONES_MASK_HPP

#include <pfss/sst/catalog/PFSS_SST_CONSTEXPR_ASSERT.hpp>
#include <pfss/sst/catalog/is_negative.hpp>
#include <pfss/sst/catalog/type_max.hpp>
#include <pfss/sst/catalog/unsigned_le.hpp>
#include <pfss/sst/catalog/value_bits.hpp>
#include <type_traits>

namespace pfss { namespace sst {

template<class IntType1,
         class IntType2,
         typename std::enable_if<std::is_integral<IntType1>()
                                     && std::is_integral<IntType2>(),
                                 int>::type = 0>
constexpr IntType1 ones_mask(IntType2 const n) noexcept {
  PFSS_SST_CONSTEXPR_ASSERT(!::pfss::sst::is_negative(n));
  PFSS_SST_CONSTEXPR_ASSERT(
      ::pfss::sst::unsigned_le(n, ::pfss::sst::value_bits<IntType1>::value));
  return n == 0 ? static_cast<IntType1>(0) :
                  static_cast<IntType1>(::pfss::sst::type_max<IntType1>()
                                        >> (::pfss::sst::value_bits<IntType1>()
                                            - static_cast<int>(n)));
}

template<class IntType1,
         class IntType2,
         class IntType3,
         typename std::enable_if<std::is_integral<IntType1>()
                                     && std::is_integral<IntType2>()
                                     && std::is_integral<IntType3>(),
                                 int>::type = 0>
constexpr IntType1 ones_mask(IntType2 const n,
                             IntType3 const k) noexcept {
  PFSS_SST_CONSTEXPR_ASSERT(!::pfss::sst::is_negative(n));
  PFSS_SST_CONSTEXPR_ASSERT(!::pfss::sst::is_negative(k));
  PFSS_SST_CONSTEXPR_ASSERT(
      ::pfss::sst::unsigned_le(n, ::pfss::sst::value_bits<IntType1>::value));
  PFSS_SST_CONSTEXPR_ASSERT(::pfss::sst::unsigned_le(k,
                                        ::pfss::sst::value_bits<IntType1>()
                                            - static_cast<int>(n)));
  return static_cast<IntType1>(::pfss::sst::ones_mask<IntType1>(n) << k);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_ONES_MASK_HPP
