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

#ifndef PFSS_SST_CATALOG_PFSS_SST_STATIC_ASSERT_HPP
#define PFSS_SST_CATALOG_PFSS_SST_STATIC_ASSERT_HPP

#include <pfss/sst/catalog/PFSS_SST_CPP11_OR_LATER.hpp>
#include <pfss/sst/catalog/PFSS_SST_CPP17_OR_LATER.hpp>

#if PFSS_SST_CPP17_OR_LATER

#define PFSS_SST_STATIC_ASSERT(x) static_assert(x)

#elif PFSS_SST_CPP11_OR_LATER

#define PFSS_SST_STATIC_ASSERT(x) static_assert(x, #x)

#else

//
// This implementation does not work at class scope, but it also does
// not have any __LINE__ collision problems. If you want to use it at
// class scope, put your assertions in a dummy function.
//

#include <pfss/sst/catalog/enable_if.hpp>

#define PFSS_SST_STATIC_ASSERT(x)                                           \
  extern int                                                           \
      PFSS_SST_STATIC_ASSERT[sizeof(typename ::pfss::sst::enable_if<(x)>::type)]

#endif

#endif // #ifndef PFSS_SST_CATALOG_PFSS_SST_STATIC_ASSERT_HPP
