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

#ifndef PFSS_SST_CATALOG_IS_EXACT_WIDTH_INTEGER_HPP
#define PFSS_SST_CATALOG_IS_EXACT_WIDTH_INTEGER_HPP

#include <cstdint>
#include <type_traits>

namespace pfss { namespace sst {

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

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_EXACT_WIDTH_INTEGER_HPP
