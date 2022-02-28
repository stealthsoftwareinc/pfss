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

#ifndef PFSS_SST_CATALOG_IS_INPUT_ITERABLE_HPP
#define PFSS_SST_CATALOG_IS_INPUT_ITERABLE_HPP

#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/begin.hpp>
#include <pfss/sst/catalog/dependent_true.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/end.hpp>
#include <pfss/sst/catalog/is_input_iterator.hpp>
#include <pfss/sst/catalog/is_sentinel_for.hpp>

namespace pfss { namespace sst {

namespace guts {
namespace is_input_iterable {

template<class, class, template<class...> class, class...>
struct x : std::false_type {};

#define PFSS_SST_c (std::declval<C &>())
#define PFSS_SST_I decltype(::pfss::sst::begin(PFSS_SST_c))
#define PFSS_SST_S decltype(::pfss::sst::end(PFSS_SST_c))

template<class C, template<class...> class T, class... Args>
struct x<
    ::pfss::sst::enable_if_t<::pfss::sst::is_input_iterator<PFSS_SST_I, T, Args...>::value
                     && ::pfss::sst::is_sentinel_for<PFSS_SST_S, PFSS_SST_I>::value>,
    C,
    T,
    Args...> : std::true_type {};

#undef PFSS_SST_S
#undef PFSS_SST_I
#undef PFSS_SST_c

} // namespace is_input_iterable
} // namespace guts

template<class C,
         template<class...> class T = ::pfss::sst::dependent_true,
         class... Args>
using is_input_iterable =
    guts::is_input_iterable::x<::pfss::sst::enable_t, C, T, Args...>;

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_IS_INPUT_ITERABLE_HPP
