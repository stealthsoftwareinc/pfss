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

#ifndef PFSS_SST_CATALOG_IS_BYTE_POINTER_HPP
#define PFSS_SST_CATALOG_IS_BYTE_POINTER_HPP

#include <pfss/sst/catalog/PFSS_SST_DEFINE_BOOLEAN_TRAIT_1.hpp>
#include <pfss/sst/catalog/is_byte.hpp>
#include <pfss/sst/catalog/remove_cvref_t.hpp>
#include <type_traits>

namespace pfss { namespace sst {

PFSS_SST_DEFINE_BOOLEAN_TRAIT_1(
    is_byte_pointer,
    T,
    (std::is_pointer<::pfss::sst::remove_cvref_t<T>>::value
     && ::pfss::sst::is_byte<typename std::remove_pointer<
         ::pfss::sst::remove_cvref_t<T>>::type>::value))

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_BYTE_POINTER_HPP
