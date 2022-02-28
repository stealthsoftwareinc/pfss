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

#ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_UNSCOPED_ENUM_HPP
#define PFSS_SST_PRIVATE_TO_STRING_FROM_UNSCOPED_ENUM_HPP

#include <iterator>
#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/integer_to_string_options.hpp>
#include <pfss/sst/catalog/is_output_iterator.hpp>
#include <pfss/sst/catalog/is_unscoped_enum.hpp>
#include <pfss/sst/catalog/to_string.hpp>
#include <pfss/sst/catalog/to_string_functor.hpp>

namespace pfss { namespace sst {

//----------------------------------------------------------------------
// src, dst, options
//----------------------------------------------------------------------

template<class X, class Src, class Dst, class Options>
class to_string_functor<
    ::pfss::sst::enable_if_t<
        ::pfss::sst::is_unscoped_enum<Src>::value
        && ::pfss::sst::is_output_iterator<Dst, char>::value
        && std::is_convertible<Options,
                               ::pfss::sst::integer_to_string_options>::value>,
    X,
    Src,
    Dst,
    Options> {

  using src_t = ::pfss::sst::decay_t<Src>;
  using dst_t = ::pfss::sst::decay_t<Dst>;
  using int_t = typename std::underlying_type<src_t>::type;

public:

  dst_t operator()(
      src_t const src,
      dst_t dst,
      ::pfss::sst::integer_to_string_options const & options = {}) const {
    return ::pfss::sst::to_string(static_cast<int_t>(src),
                          std::move(dst),
                          options);
  }

}; //

//----------------------------------------------------------------------
// src, dst
//----------------------------------------------------------------------

template<class X, class Src, class Dst>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_unscoped_enum<Src>::value
                     && ::pfss::sst::is_output_iterator<Dst, char>::value>,
    X,
    Src,
    Dst> : public to_string_functor<::pfss::sst::enable_t,
                                    X,
                                    Src,
                                    Dst,
                                    ::pfss::sst::integer_to_string_options> {};

//----------------------------------------------------------------------
// src, options
//----------------------------------------------------------------------

template<class Dst, class Src, class Options>
class to_string_functor<
    ::pfss::sst::enable_if_t<
        ::pfss::sst::is_unscoped_enum<Src>::value
        && std::is_convertible<Options,
                               ::pfss::sst::integer_to_string_options>::value>,
    Dst,
    Src,
    Options> {

  using src_t = ::pfss::sst::decay_t<Src>;

public:

  ::pfss::sst::decay_t<Dst> operator()(
      src_t const src,
      ::pfss::sst::integer_to_string_options const & options = {}) const {
    ::pfss::sst::decay_t<Dst> dst;
    ::pfss::sst::to_string(src, std::back_inserter(dst), options);
    return dst;
  }

}; //

//----------------------------------------------------------------------
// src
//----------------------------------------------------------------------

template<class X, class Src>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_unscoped_enum<Src>::value>,
    X,
    Src> : public to_string_functor<::pfss::sst::enable_t,
                                    X,
                                    Src,
                                    ::pfss::sst::integer_to_string_options> {};

//----------------------------------------------------------------------

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_UNSCOPED_ENUM_HPP
