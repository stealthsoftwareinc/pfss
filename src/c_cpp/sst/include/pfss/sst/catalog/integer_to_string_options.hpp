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

#ifndef PFSS_SST_CATALOG_INTEGER_TO_STRING_OPTIONS_HPP
#define PFSS_SST_CATALOG_INTEGER_TO_STRING_OPTIONS_HPP

#include <pfss/sst/catalog/PFSS_SST_ASSERT.hpp>

namespace pfss { namespace sst {

class integer_to_string_options final {

  int base_ = 10;
  bool prefix_ = false;
  bool uppercase_digits_ = true;
  bool uppercase_prefix_ = false;

public:

  //--------------------------------------------------------------------
  // base
  //--------------------------------------------------------------------

  int base() const {
    return base_;
  }

  integer_to_string_options & base(int const x) {
    PFSS_SST_ASSERT((x >= 0));
    PFSS_SST_ASSERT((x <= 36));
    base_ = x;
    return *this;
  }

  //--------------------------------------------------------------------
  // prefix
  //--------------------------------------------------------------------

  bool prefix() const {
    return prefix_;
  }

  integer_to_string_options & prefix(bool const x) {
    prefix_ = x;
    return *this;
  }

  //--------------------------------------------------------------------
  // uppercase_digits
  //--------------------------------------------------------------------

  bool uppercase_digits() const {
    return uppercase_digits_;
  }

  integer_to_string_options & uppercase_digits(bool const x) {
    uppercase_digits_ = x;
    return *this;
  }

  //--------------------------------------------------------------------
  // uppercase_prefix
  //--------------------------------------------------------------------

  bool uppercase_prefix() const {
    return uppercase_prefix_;
  }

  integer_to_string_options & uppercase_prefix(bool const x) {
    uppercase_prefix_ = x;
    return *this;
  }

  //--------------------------------------------------------------------
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_INTEGER_TO_STRING_OPTIONS_HPP
