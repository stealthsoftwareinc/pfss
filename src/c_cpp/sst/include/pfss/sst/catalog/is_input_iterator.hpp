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

#ifndef PFSS_SST_CATALOG_IS_INPUT_ITERATOR_HPP
#define PFSS_SST_CATALOG_IS_INPUT_ITERATOR_HPP

#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/dependent_true.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/is_iterator.hpp>
#include <pfss/sst/catalog/remove_reference_t.hpp>

namespace pfss { namespace sst {

namespace guts {
namespace is_input_iterator {

template<class, class, template<class...> class, class...>
struct x : std::false_type {};

#define PFSS_SST_I ::pfss::sst::decay_t<I>
#define PFSS_SST_i (std::declval<PFSS_SST_I &>())
#define PFSS_SST_D decltype(*PFSS_SST_i)
#define PFSS_SST_V ::pfss::sst::decay_t<PFSS_SST_D>

template<class I, template<class...> class T, class... Args>
struct x<::pfss::sst::enable_if_t<
             ::pfss::sst::is_iterator<I>::value
             && std::is_lvalue_reference<PFSS_SST_D>::value
             && std::is_object<::pfss::sst::remove_reference_t<PFSS_SST_D>>::value
             && T<PFSS_SST_V, Args...>::value>,
         I,
         T,
         Args...> : std::true_type {};

#undef PFSS_SST_V
#undef PFSS_SST_D
#undef PFSS_SST_i
#undef PFSS_SST_I

} // namespace is_input_iterator
} // namespace guts

template<class I,
         template<class...> class T = ::pfss::sst::dependent_true,
         class... Args>
using is_input_iterator =
    guts::is_input_iterator::x<::pfss::sst::enable_t, I, T, Args...>;

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_INPUT_ITERATOR_HPP
