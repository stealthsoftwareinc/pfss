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

#ifndef PFSS_SST_CATALOG_IS_BYTE_HPP
#define PFSS_SST_CATALOG_IS_BYTE_HPP

#include <type_traits>

#include <pfss/sst/catalog/PFSS_SST_CPP17_OR_LATER.hpp>
#include <pfss/sst/catalog/PFSS_SST_DEFINE_BOOLEAN_TRAIT_1.hpp>
#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/dependent_false.hpp>

#if PFSS_SST_CPP17_OR_LATER
#include <cstddef>
#endif

namespace pfss { namespace sst {

#define PFSS_SST_F(X) (std::is_same<::pfss::sst::decay_t<T>, X>::value)

namespace guts {
namespace is_byte {

#if PFSS_SST_CPP17_OR_LATER
PFSS_SST_DEFINE_BOOLEAN_TRAIT_1(f1, T, (PFSS_SST_F(std::byte)))
#else
PFSS_SST_DEFINE_BOOLEAN_TRAIT_1(f1, T, (::pfss::sst::dependent_false<T>::value))
#endif

} // namespace is_byte
} // namespace guts

PFSS_SST_DEFINE_BOOLEAN_TRAIT_1(is_byte,
                           T,
                           (PFSS_SST_F(char) || PFSS_SST_F(signed char)
                            || PFSS_SST_F(unsigned char)
                            || guts::is_byte::f1<T>::value))

#undef PFSS_SST_F

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_BYTE_HPP
