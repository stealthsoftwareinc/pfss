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

#ifndef PFSS_SST_CATALOG_MOVED_FROM_HPP
#define PFSS_SST_CATALOG_MOVED_FROM_HPP

namespace pfss { namespace sst {

class moved_from {

  bool b_;

public:

  //--------------------------------------------------------------------

  moved_from(bool const b) noexcept : b_(b) {
  }

  //--------------------------------------------------------------------

  moved_from() noexcept : moved_from(false) {
  }

  //--------------------------------------------------------------------

  moved_from(moved_from const &) noexcept = default;

  //--------------------------------------------------------------------

  moved_from & operator=(moved_from const &) noexcept = default;

  //--------------------------------------------------------------------

  moved_from(moved_from && other) noexcept : b_(other.b_) {
    other.b_ = true;
  }

  //--------------------------------------------------------------------

  moved_from & operator=(moved_from && other) noexcept {
    b_ = other.b_;
    other.b_ = true;
    return *this;
  }

  //--------------------------------------------------------------------

  ~moved_from() noexcept = default;

  //--------------------------------------------------------------------

  operator bool() const noexcept {
    return b_;
  }

  //--------------------------------------------------------------------
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_MOVED_FROM_HPP
