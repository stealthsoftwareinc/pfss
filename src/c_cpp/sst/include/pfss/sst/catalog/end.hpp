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

#ifndef PFSS_SST_CATALOG_END_HPP
#define PFSS_SST_CATALOG_END_HPP

#include <utility>

namespace PFSS_SST_GUTS {
namespace end {

using std::end;

#define PFSS_SST_X (end(std::forward<C>(c)))

template<class C>
auto f(C && c) noexcept(noexcept(PFSS_SST_X)) -> decltype(PFSS_SST_X) {
  return PFSS_SST_X;
}

#undef PFSS_SST_X

} // namespace end
} // namespace PFSS_SST_GUTS

namespace pfss { namespace sst {

namespace guts {
namespace end {

#define PFSS_SST_X (PFSS_SST_GUTS::end::f(std::forward<C>(c)))

template<class C>
auto end(C && c) noexcept(noexcept(PFSS_SST_X)) -> decltype(PFSS_SST_X) {
  return PFSS_SST_X;
}

#undef PFSS_SST_X

} // namespace end
} // namespace guts

// "using namespace" is what prevents ADL.
using namespace guts::end;

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_END_HPP
