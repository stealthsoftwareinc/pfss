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

#ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_CHAR_INPUT_ITERABLE_HPP
#define PFSS_SST_PRIVATE_TO_STRING_FROM_CHAR_INPUT_ITERABLE_HPP

#include <iterator>
#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/cbegin.hpp>
#include <pfss/sst/catalog/cend.hpp>
#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/is_bool.hpp>
#include <pfss/sst/catalog/is_char_input_iterable.hpp>
#include <pfss/sst/catalog/is_output_iterator.hpp>
#include <pfss/sst/catalog/to_string.hpp>
#include <pfss/sst/catalog/to_string_functor.hpp>
#include <pfss/sst/catalog/value_type_t.hpp>

namespace pfss { namespace sst {

//----------------------------------------------------------------------
// src, dst, replace
//----------------------------------------------------------------------

template<class DstValue, class Src, class Dst, class Replace>
class to_string_functor<
    ::pfss::sst::enable_if_t<
        ::pfss::sst::is_char_input_iterable<::pfss::sst::decay_t<Src>>::value
        && ::pfss::sst::is_output_iterator<Dst>::value
        && ::pfss::sst::is_bool<Replace>::value>,
    DstValue,
    Src,
    Dst,
    Replace> {

  using src_t = ::pfss::sst::decay_t<Src>;
  using dst_t = ::pfss::sst::decay_t<Dst>;

public:

  dst_t operator()(src_t const & src,
                   dst_t dst,
                   bool const replace = false) const {
    return ::pfss::sst::to_string<DstValue>(::pfss::sst::cbegin(src),
                                    ::pfss::sst::cend(src),
                                    std::move(dst),
                                    replace);
  }

}; //

//----------------------------------------------------------------------
// src, dst
//----------------------------------------------------------------------

template<class X, class Src, class Dst>
class to_string_functor<
    ::pfss::sst::enable_if_t<
        ::pfss::sst::is_char_input_iterable<::pfss::sst::decay_t<Src>>::value
        && ::pfss::sst::is_output_iterator<Dst>::value>,
    X,
    Src,
    Dst> : public to_string_functor<::pfss::sst::enable_t, X, Src, Dst, bool> {
};

//----------------------------------------------------------------------
// src, replace
//----------------------------------------------------------------------

template<class Dst, class Src, class Replace>
class to_string_functor<
    ::pfss::sst::enable_if_t<
        ::pfss::sst::is_char_input_iterable<::pfss::sst::decay_t<Src>>::value
        && ::pfss::sst::is_bool<Replace>::value>,
    Dst,
    Src,
    Replace> {

  using src_t = ::pfss::sst::decay_t<Src>;

public:

  ::pfss::sst::decay_t<Dst> operator()(src_t const & src,
                               bool const replace = false) const {
    ::pfss::sst::decay_t<Dst> dst;
    using dst_value_t = ::pfss::sst::value_type_t<decltype(dst)>;
    ::pfss::sst::to_string<dst_value_t>(src, std::back_inserter(dst), replace);
    return dst;
  }

}; //

//----------------------------------------------------------------------
// src
//----------------------------------------------------------------------

template<class X, class Src>
class to_string_functor<::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterable<
                            ::pfss::sst::decay_t<Src>>::value>,
                        X,
                        Src>
    : public to_string_functor<::pfss::sst::enable_t, X, Src, bool> {};

//----------------------------------------------------------------------

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_CHAR_INPUT_ITERABLE_HPP
