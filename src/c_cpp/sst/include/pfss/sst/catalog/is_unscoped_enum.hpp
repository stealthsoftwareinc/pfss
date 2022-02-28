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

#ifndef PFSS_SST_CATALOG_IS_UNSCOPED_ENUM_HPP
#define PFSS_SST_CATALOG_IS_UNSCOPED_ENUM_HPP

#include <type_traits>

#include <pfss/sst/catalog/PFSS_SST_DEFINE_BOOLEAN_TRAIT_1.hpp>
#include <pfss/sst/catalog/is_enum.hpp>
#include <pfss/sst/catalog/underlying_type_t.hpp>

namespace pfss { namespace sst {

PFSS_SST_DEFINE_BOOLEAN_TRAIT_1(
    is_unscoped_enum,
    T,
    (::pfss::sst::is_enum<T>::value
     && std::is_convertible<T, ::pfss::sst::underlying_type_t<T>>::value))

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_UNSCOPED_ENUM_HPP
