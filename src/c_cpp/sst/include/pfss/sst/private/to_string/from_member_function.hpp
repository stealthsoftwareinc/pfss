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

#ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_MEMBER_FUNCTION_HPP
#define PFSS_SST_PRIVATE_TO_STRING_FROM_MEMBER_FUNCTION_HPP

#include <utility>

#include <pfss/sst/catalog/PFSS_SST_COMPILES.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/to_string.hpp>
#include <pfss/sst/catalog/to_string_functor.hpp>

namespace pfss { namespace sst {

//----------------------------------------------------------------------
// src, args...
//----------------------------------------------------------------------

template<class X, class Src, class... Args>
class to_string_functor<
    ::pfss::sst::enable_if_t<PFSS_SST_COMPILES(
        (std::declval<Src const &>().template to_string<X>(
            std::forward<Args>(std::declval<Args>())...)))>,
    X,
    Src,
    Args...> {

public:

#define PFSS_SST_X (src.template to_string<X>(std::forward<Args2>(args)...))

  template<class... Args2>
  auto operator()(Src const & src, Args2 &&... args) const
      noexcept(noexcept(PFSS_SST_X)) -> decltype(PFSS_SST_X) {
    return PFSS_SST_X;
  }

#undef PFSS_SST_X

}; //

//----------------------------------------------------------------------

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_MEMBER_FUNCTION_HPP
