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

#ifndef PFSS_SST_CATALOG_IS_REPRESENTABLE_AS_HPP
#define PFSS_SST_CATALOG_IS_REPRESENTABLE_AS_HPP

#include <pfss/sst/catalog/can_represent_all.hpp>
#include <pfss/sst/catalog/perfect_le.hpp>
#include <pfss/sst/catalog/remove_cvref.hpp>

namespace pfss { namespace sst {

template<class A, class B>
constexpr bool is_representable_as(B const b) noexcept {
  using A_ = typename ::pfss::sst::remove_cvref<A>::type;
  using B_ = typename ::pfss::sst::remove_cvref<B>::type;
  return ::pfss::sst::can_represent_all<A_, B_>::value ?
             true :
             ::pfss::sst::perfect_le(::pfss::sst::type_min<A_>::value, b)
                 && ::pfss::sst::perfect_le(b, ::pfss::sst::type_max<A_>::value);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_REPRESENTABLE_AS_HPP
