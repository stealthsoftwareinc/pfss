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

#ifndef PFSS_SST_CATALOG_IS_LITTLE_ENDIAN_HPP
#define PFSS_SST_CATALOG_IS_LITTLE_ENDIAN_HPP

#include <pfss/sst/PFSS_SST_CPP_OR_LATER.hpp>
#include <pfss/sst/type.h>
#include <type_traits>

#if PFSS_SST_CPP20_OR_LATER
#include <bit>
#endif

namespace pfss { namespace sst {

template<class, class = void>
struct is_little_endian : std::false_type {};

template<class T>
struct is_little_endian<
    T,
    typename std::enable_if<
        false
        || (::pfss::sst::is_non_bool_integer<T>::value
            && (sizeof(T) == 1
#if PFSS_SST_CPP20_OR_LATER
                || std::endian::native == std::endian::little
#endif
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
                || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#endif
                ))>::type> : std::true_type {
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_LITTLE_ENDIAN_HPP
