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

#ifndef PFSS_SST_CATALOG_PERFECT_LT_HPP
#define PFSS_SST_CATALOG_PERFECT_LT_HPP

#include <type_traits>

#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/is_integer.hpp>
#include <pfss/sst/catalog/is_negative.hpp>
#include <pfss/sst/catalog/promote.hpp>
#include <pfss/sst/catalog/remove_cvref_t.hpp>
#include <pfss/sst/catalog/unsigned_lt.hpp>

namespace pfss { namespace sst {

template<class T1,
         class T2,
         typename ::pfss::sst::enable_if_t<::pfss::sst::is_integer<T1>::value
                                   && ::pfss::sst::is_integer<T2>::value> = 0>
constexpr bool perfect_lt(T1 const x, T2 const y) noexcept {
  using P1 = typename ::pfss::sst::promote<::pfss::sst::remove_cvref_t<T1>>::type;
  using P2 = typename ::pfss::sst::promote<::pfss::sst::remove_cvref_t<T2>>::type;
  using S1 = typename std::make_signed<P1>::type;
  using S2 = typename std::make_signed<P2>::type;
  return ::pfss::sst::is_negative(x) ?
             !::pfss::sst::is_negative(y)
                 || static_cast<S1>(x) < static_cast<S2>(y) :
             !::pfss::sst::is_negative(y) && ::pfss::sst::unsigned_lt(x, y);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_PERFECT_LT_HPP
