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

#ifndef PFSS_SST_CATALOG_ENABLE_IF_HPP
#define PFSS_SST_CATALOG_ENABLE_IF_HPP

#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/type_identity.hpp>

namespace pfss { namespace sst {

template<bool, class = ::pfss::sst::enable_t>
struct enable_if;

template<class T>
struct enable_if<true, T> : ::pfss::sst::type_identity<T> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_ENABLE_IF_HPP
