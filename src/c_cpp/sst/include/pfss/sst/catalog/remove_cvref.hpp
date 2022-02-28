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

#ifndef PFSS_SST_CATALOG_REMOVE_CVREF_HPP
#define PFSS_SST_CATALOG_REMOVE_CVREF_HPP

#include <type_traits>

#include <pfss/sst/catalog/type_identity.hpp>

namespace pfss { namespace sst {

namespace guts {
namespace remove_cvref {

template<class T>
struct x {
  using R1 = T;
  using R2 = typename std::remove_reference<R1>::type;
  using R3 = typename std::remove_cv<R2>::type;
  using type = R3;
};

template<class T>
using x_t = typename x<T>::type;

} // namespace remove_cvref
} // namespace guts

template<class T>
struct remove_cvref
    : ::pfss::sst::type_identity<::pfss::sst::guts::remove_cvref::x_t<T>> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_REMOVE_CVREF_HPP
