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

#ifndef PFSS_SST_CATALOG_VALUE_BITS_HPP
#define PFSS_SST_CATALOG_VALUE_BITS_HPP

#include <limits>
#include <type_traits>

namespace pfss { namespace sst {

template<class IntType, class = void>
struct value_bits;

template<class IntType>
struct value_bits<
    IntType,
    typename std::enable_if<std::is_integral<IntType>::value>::type>
    : std::integral_constant<int,
                             std::numeric_limits<IntType>::digits> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_VALUE_BITS_HPP
