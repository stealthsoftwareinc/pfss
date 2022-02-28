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

#ifndef PFSS_SST_CATALOG_CHECKED_CAST_HPP
#define PFSS_SST_CATALOG_CHECKED_CAST_HPP

#include <pfss/sst/catalog/PFSS_SST_CPP14_CONSTEXPR.hpp>
#include <pfss/sst/catalog/checked.hpp>
#include <pfss/sst/catalog/checked_overflow.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/is_bigint.hpp>
#include <pfss/sst/catalog/type_max.hpp>
#include <pfss/sst/catalog/type_min.hpp>
#include <type_traits>

namespace pfss { namespace sst {

template<class T,
         class U,
         ::pfss::sst::enable_if_t<std::is_integral<T>::value
                          && std::is_integral<U>::value> = 0>
PFSS_SST_CPP14_CONSTEXPR T checked_cast(U const x) {
  return ::pfss::sst::checked<T>(x).value();
}

template<class T,
         class U,
         ::pfss::sst::enable_if_t<std::is_integral<T>::value
                          && std::is_integral<U>::value> = 0>
PFSS_SST_CPP14_CONSTEXPR T checked_cast(::pfss::sst::checked<U> const x) {
  return ::pfss::sst::checked<T>(x).value();
}

template<class T,
         class U,
         ::pfss::sst::enable_if_t<std::is_integral<T>::value
                          && ::pfss::sst::is_bigint<U>::value> = 0>
T checked_cast(U const & x) {
  if (x < ::pfss::sst::type_min<T>::value || x > ::pfss::sst::type_max<T>::value) {
    throw ::pfss::sst::checked_overflow("::pfss::sst::checked_cast overflow");
  }
  return static_cast<T>(x);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_CHECKED_CAST_HPP
