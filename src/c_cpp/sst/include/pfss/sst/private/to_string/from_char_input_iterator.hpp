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

#ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_CHAR_INPUT_ITERATOR_HPP
#define PFSS_SST_PRIVATE_TO_STRING_FROM_CHAR_INPUT_ITERATOR_HPP

#include <iterator>
#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/PFSS_SST_ASSERT.hpp>
#include <pfss/sst/catalog/checked_cast.hpp>
#include <pfss/sst/catalog/decay_t.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/is_bool.hpp>
#include <pfss/sst/catalog/is_char_input_iterator.hpp>
#include <pfss/sst/catalog/is_integer.hpp>
#include <pfss/sst/catalog/is_output_iterator.hpp>
#include <pfss/sst/catalog/is_representable_as.hpp>
#include <pfss/sst/catalog/is_sentinel_for.hpp>
#include <pfss/sst/catalog/to_string.hpp>
#include <pfss/sst/catalog/to_string_functor.hpp>
#include <pfss/sst/catalog/value_type_t.hpp>

namespace pfss { namespace sst {

//----------------------------------------------------------------------
// src, end, dst, replace
//----------------------------------------------------------------------

template<class DstValue, class Src, class End, class Dst, class Replace>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterator<Src>::value
                     && ::pfss::sst::is_sentinel_for<End, Src>::value
                     && ::pfss::sst::is_output_iterator<Dst>::value
                     && ::pfss::sst::is_bool<Replace>::value>,
    DstValue,
    Src,
    End,
    Dst,
    Replace> {

  using src_t = ::pfss::sst::decay_t<Src>;
  using end_t = ::pfss::sst::decay_t<End>;
  using dst_t = ::pfss::sst::decay_t<Dst>;
  using src_value_t = ::pfss::sst::value_type_t<src_t>;

public:

  dst_t operator()(src_t src,
                   end_t const end,
                   dst_t dst,
                   bool const replace = false) const {
    using dst_value_t =
        typename std::conditional<::pfss::sst::is_integer<DstValue>::value,
                                  DstValue,
                                  ::pfss::sst::value_type_t<dst_t>>::type;
    // clang-format off
    PFSS_SST_STATIC_ASSERT("When the output value type is not overridden, the value type of the output iterator must be an integer type." && (::pfss::sst::is_integer<dst_value_t>::value));
    // clang-format on
    if (replace) {
      for (; src != end; ++src) {
        src_value_t const c = *src;
        if (::pfss::sst::is_representable_as<dst_value_t>(c)) {
          *dst = static_cast<dst_value_t>(c);
        } else {
          *dst = static_cast<dst_value_t>('?');
        }
        ++dst;
      }
    } else {
      for (; src != end; ++src) {
        src_value_t const c = *src;
        *dst = ::pfss::sst::checked_cast<dst_value_t>(c);
        ++dst;
      }
    }
    return dst;
  }

}; //

//----------------------------------------------------------------------
// src, end, dst
//----------------------------------------------------------------------

template<class X, class Src, class End, class Dst>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterator<Src>::value
                     && ::pfss::sst::is_sentinel_for<End, Src>::value
                     && ::pfss::sst::is_output_iterator<Dst>::value>,
    X,
    Src,
    End,
    Dst>
    : public to_string_functor<::pfss::sst::enable_t, X, Src, End, Dst, bool> {
};

//----------------------------------------------------------------------
// src, end, bool
//----------------------------------------------------------------------

template<class Dst, class Src, class End, class Replace>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterator<Src>::value
                     && ::pfss::sst::is_sentinel_for<End, Src>::value
                     && ::pfss::sst::is_bool<Replace>::value>,
    Dst,
    Src,
    End,
    Replace> {

  using src_t = ::pfss::sst::decay_t<Src>;
  using end_t = ::pfss::sst::decay_t<End>;
  using src_value_t = ::pfss::sst::value_type_t<src_t>;

public:

  ::pfss::sst::decay_t<Dst>
  operator()(src_t src, end_t end, bool const replace = false) const {
    ::pfss::sst::decay_t<Dst> dst;
    using dst_value_t = ::pfss::sst::value_type_t<decltype(dst)>;
    ::pfss::sst::to_string<dst_value_t>(std::move(src),
                                std::move(end),
                                std::back_inserter(dst),
                                replace);
    return dst;
  }

}; //

//----------------------------------------------------------------------
// src, end
//----------------------------------------------------------------------

template<class X, class Src, class End>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterator<Src>::value
                     && ::pfss::sst::is_sentinel_for<End, Src>::value>,
    X,
    Src,
    End> : public to_string_functor<::pfss::sst::enable_t, X, Src, End, bool> {
};

//----------------------------------------------------------------------
// src, bool
//----------------------------------------------------------------------

template<class Dst, class Src, class Replace>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterator<Src>::value
                     && ::pfss::sst::is_bool<Replace>::value>,
    Dst,
    Src,
    Replace> {

  using src_t = ::pfss::sst::decay_t<Src>;
  using src_value_t = ::pfss::sst::value_type_t<src_t>;

public:

  ::pfss::sst::decay_t<Dst> operator()(src_t src,
                               bool const replace = false) const {
    ::pfss::sst::decay_t<Dst> dst;
    using dst_value_t = ::pfss::sst::value_type_t<decltype(dst)>;
    if (replace) {
      for (;; ++src) {
        src_value_t const c = *src;
        if (c == 0) {
          return dst;
        }
        if (::pfss::sst::is_representable_as<dst_value_t>(c)) {
          *dst = static_cast<dst_value_t>(c);
        } else {
          *dst = static_cast<dst_value_t>('?');
        }
        ++dst;
      }
    } else {
      for (;; ++src) {
        src_value_t const c = *src;
        if (c == 0) {
          return dst;
        }
        *dst = ::pfss::sst::checked_cast<dst_value_t>(c);
        ++dst;
      }
    }
  }

}; //

//----------------------------------------------------------------------
// src
//----------------------------------------------------------------------

template<class X, class Src>
class to_string_functor<
    ::pfss::sst::enable_if_t<::pfss::sst::is_char_input_iterator<Src>::value>,
    X,
    Src> : public to_string_functor<::pfss::sst::enable_t, X, Src, bool> {};

//----------------------------------------------------------------------

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_PRIVATE_TO_STRING_FROM_CHAR_INPUT_ITERATOR_HPP
