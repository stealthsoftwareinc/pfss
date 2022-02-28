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

#ifndef PFSS_SST_CATALOG_COPY_CV_HPP
#define PFSS_SST_CATALOG_COPY_CV_HPP

#include <type_traits>

#include <pfss/sst/catalog/type_identity.hpp>

namespace pfss { namespace sst {

namespace guts {
namespace copy_cv {

template<class T1, class T2>
struct x {
  using R1 = T2;
  using R2 = typename std::conditional<
      std::is_const<T1>::value,
      R1 const,
      typename std::remove_const<R1>::type>::type;
  using R3 = typename std::conditional<
      std::is_volatile<T1>::value,
      R2 volatile,
      typename std::remove_volatile<R2>::type>::type;
  using type = R3;
};

template<class T1, class T2>
using x_t = typename x<T1, T2>::type;

} // namespace copy_cv
} // namespace guts

template<class T1, class T2>
struct copy_cv : ::pfss::sst::type_identity<::pfss::sst::guts::copy_cv::x_t<T1, T2>> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_COPY_CV_HPP
