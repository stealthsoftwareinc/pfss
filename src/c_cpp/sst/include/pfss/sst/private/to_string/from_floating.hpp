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

#ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_FLOATING_HPP
#define PFSS_SST_PRIVATE_TO_STRING_FROM_FLOATING_HPP

#include <iterator>
#include <locale>
#include <sstream>
#include <string>

#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/is_floating.hpp>
#include <pfss/sst/catalog/is_output_iterator.hpp>

namespace pfss { namespace sst {

//----------------------------------------------------------------------
// src, dst
//----------------------------------------------------------------------

template<class X, class Src, class Dst>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_floating<Src>::value
                     && ::pfss::sst::is_output_iterator<Dst, char>::value>,
    X,
    Src,
    Dst> {

  using src_t = ::pfss::sst::decay_t<Src>;
  using dst_t = ::pfss::sst::decay_t<Dst>;

public:

  dst_t operator()(src_t const src, dst_t dst) const {
    std::ostringstream a;
    a.imbue(std::locale::classic());
    a << src;
    std::string const b = a.str();
    for (char const c : b) {
      *dst = c;
      ++dst;
    }
    return dst;
  }

}; //

//----------------------------------------------------------------------
// src
//----------------------------------------------------------------------

template<class Dst, class Src>
class to_string_functor<::pfss::sst::enable_if_t<::pfss::sst::is_floating<Src>::value>,
                        Dst,
                        Src> {

  using src_t = ::pfss::sst::decay_t<Src>;

public:

  ::pfss::sst::decay_t<Dst> operator()(src_t const src) const {
    ::pfss::sst::decay_t<Dst> dst;
    ::pfss::sst::to_string(src, std::back_inserter(dst));
    return dst;
  }

}; //

//----------------------------------------------------------------------

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_FLOATING_HPP
