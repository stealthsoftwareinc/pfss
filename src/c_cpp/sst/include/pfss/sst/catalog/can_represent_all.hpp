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

#ifndef PFSS_SST_CATALOG_CAN_REPRESENT_ALL_HPP
#define PFSS_SST_CATALOG_CAN_REPRESENT_ALL_HPP

#include <pfss/sst/catalog/perfect_ge.hpp>
#include <pfss/sst/catalog/perfect_le.hpp>
#include <pfss/sst/catalog/type_max.hpp>
#include <pfss/sst/catalog/type_min.hpp>
#include <type_traits>

namespace pfss { namespace sst {

//
// ::pfss::sst::can_represent_all<T, U>::value indicates whether an integer type
// T can represent all values of an integer type U.
//
// This trait is named after the "can represent all" phrasing used in
// the C and C++ standards when discussing the ranges of integer types.
//

namespace detail {

template<class T, class U, class = void>
struct can_represent_all_2 : std::false_type {};

template<class T, class U>
struct can_represent_all_2<
    T,
    U,
    typename std::enable_if<::pfss::sst::perfect_le(::pfss::sst::type_min<T>::value,
                                            ::pfss::sst::type_min<U>::value)
                            && ::pfss::sst::perfect_ge(
                                ::pfss::sst::type_max<T>::value,
                                ::pfss::sst::type_max<U>::value)>::type>
    : std::true_type {};

} // namespace detail

template<class T, class U, class = void>
struct can_represent_all : std::false_type {};

//
// Why is std::remove_cv used here? Because the
// src/c-cpp/test/sst/can_represent_all.cpp unit test in SST commit
// a9c55b7791023bbf0f1a7eae215b150748f5bc6c was failing on CentOS 7 on
// 2020-09-12 (g++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-39)). GCC seems
// to have a buggy bool volatile. The extent of the issue is unclear,
// but std::numeric_limits<bool volatile>::max() at least sometimes
// yields 0. Using std::remove_cv here helps the unit test succeed
// without affecting the SST interface.
//

template<class T, class U>
struct can_represent_all<
    T,
    U,
    typename std::enable_if<std::is_integral<T>()
                            && std::is_integral<U>()>::type>
    : detail::can_represent_all_2<typename std::remove_cv<T>::type,
                                  typename std::remove_cv<U>::type> {};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_CAN_REPRESENT_ALL_HPP
