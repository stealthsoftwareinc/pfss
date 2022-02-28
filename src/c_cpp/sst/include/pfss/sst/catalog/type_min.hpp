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

#ifndef PFSS_SST_CATALOG_TYPE_MIN_HPP
#define PFSS_SST_CATALOG_TYPE_MIN_HPP

#include <limits>
#include <pfss/sst/catalog/PFSS_SST_STATIC_ASSERT.hpp>
#include <type_traits>

namespace pfss { namespace sst {

template<class T, class = void>
struct type_min;

template<class T>
struct type_min<
    T,
    typename std::enable_if<std::is_integral<T>::value>::type>
    : std::integral_constant<T, std::numeric_limits<T>::min()> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_TYPE_MIN_HPP
