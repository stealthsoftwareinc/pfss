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

#ifndef PFSS_SST_CATALOG_MONO_TIME_HPP
#define PFSS_SST_CATALOG_MONO_TIME_HPP

#include <chrono>
#include <pfss/sst/catalog/mono_time_zero.hpp>
#include <pfss/sst/private/PFSS_SST_DLL_EXPORT.h>

namespace pfss { namespace sst {

template<class Duration>
Duration mono_time() {
  return std::chrono::duration_cast<Duration>(
      std::chrono::steady_clock::now() - ::pfss::sst::mono_time_zero());
}

extern template PFSS_SST_DLL_EXPORT std::chrono::seconds mono_time<std::chrono::seconds>();

extern template PFSS_SST_DLL_EXPORT std::chrono::milliseconds
mono_time<std::chrono::milliseconds>();

extern template PFSS_SST_DLL_EXPORT std::chrono::microseconds
mono_time<std::chrono::microseconds>();

extern template PFSS_SST_DLL_EXPORT std::chrono::nanoseconds
mono_time<std::chrono::nanoseconds>();

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_MONO_TIME_HPP
