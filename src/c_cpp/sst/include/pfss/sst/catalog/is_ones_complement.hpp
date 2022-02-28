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

#ifndef PFSS_SST_CATALOG_IS_ONES_COMPLEMENT_HPP
#define PFSS_SST_CATALOG_IS_ONES_COMPLEMENT_HPP

#include <type_traits>

#include <pfss/sst/catalog/PFSS_SST_STATIC_ASSERT.hpp>
#include <pfss/sst/catalog/dependent_false.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/is_signed_integer.hpp>
#include <pfss/sst/catalog/is_twos_complement.hpp>
#include <pfss/sst/catalog/promotes.hpp>

namespace pfss { namespace sst {

template<class T, class = ::pfss::sst::enable_t>
struct is_ones_complement : std::false_type {};

template<class T>
struct is_ones_complement<
    T,
    ::pfss::sst::enable_if_t<::pfss::sst::is_signed_integer<T>::value
                     && !::pfss::sst::is_twos_complement<T>::value
                     && !::pfss::sst::promotes<T>::value
                     && (static_cast<T>(-1) & static_cast<T>(1)) == 0>>
    : std::true_type {};

template<class T>
struct is_ones_complement<
    T,
    ::pfss::sst::enable_if_t<::pfss::sst::is_signed_integer<T>::value
                     && !::pfss::sst::is_twos_complement<T>::value
                     && ::pfss::sst::promotes<T>::value>> {
  // clang-format off
  PFSS_SST_STATIC_ASSERT(("It cannot be determined whether a signed integer type that is affected by the integer promotions and does not use two's complement representation uses ones' complement vs. sign-magnitude representation." && ::pfss::sst::dependent_false<T>::value));
  // clang-format on
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_ONES_COMPLEMENT_HPP
