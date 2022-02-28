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

#ifndef PFSS_SST_CATALOG_IS_ITERATOR_HPP
#define PFSS_SST_CATALOG_IS_ITERATOR_HPP

#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/PFSS_SST_COMPILES.hpp>
#include <pfss/sst/catalog/PFSS_SST_DEFINE_BOOLEAN_TRAIT_1.hpp>
#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/is_bool.hpp>

namespace pfss { namespace sst {

//
// Some compilers output warnings about ++x when x has type bool, as
// incrementing a bool was deprecated in C++98 and removed in C++17. We
// don't care about that warning here because I = bool will SFINAE fail
// anyway. We silence the warning by doing the is_iterator check in two
// stages: discard bool immediately, then do the actual check we'd like
// to have done in the first place.
//

namespace guts {
namespace is_iterator {

#define PFSS_SST_I ::pfss::sst::decay_t<I>
#define PFSS_SST_i (std::declval<PFSS_SST_I &>())

PFSS_SST_DEFINE_BOOLEAN_TRAIT_1(
    is_iterator,
    I,
    (PFSS_SST_COMPILES(*PFSS_SST_i)
     && std::is_same<decltype(++PFSS_SST_i), PFSS_SST_I &>::value))

#undef PFSS_SST_i
#undef PFSS_SST_I

} // namespace is_iterator
} // namespace guts

template<class I, class = ::pfss::sst::enable_t>
struct is_iterator : std::false_type {};

template<class I>
struct is_iterator<I, ::pfss::sst::enable_if_t<!::pfss::sst::is_bool<I>::value>>
    : guts::is_iterator::is_iterator<I> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_ITERATOR_HPP
