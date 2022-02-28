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

#ifndef PFSS_SST_CATALOG_PROMOTE_UNSIGNED_HPP
#define PFSS_SST_CATALOG_PROMOTE_UNSIGNED_HPP

#include <type_traits>

#include <pfss/sst/catalog/PFSS_SST_STATIC_ASSERT.hpp>
#include <pfss/sst/catalog/promote.hpp>
#include <pfss/sst/language.h>

namespace pfss { namespace sst {

template<class T>
class promote_unsigned {
  PFSS_SST_STATIC_ASSERT(std::is_integral<T>::value);

  using T1 = typename ::pfss::sst::promote<T>::type;
  using T2 = typename std::make_unsigned<T1>::type;
  using T3 =
      typename std::conditional<std::is_const<T>::value,
                                typename std::add_const<T2>::type,
                                T2>::type;
  using T4 =
      typename std::conditional<std::is_volatile<T>::value,
                                typename std::add_volatile<T3>::type,
                                T3>::type;

public:
  using type = T4;
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_PROMOTE_UNSIGNED_HPP
