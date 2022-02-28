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

#ifndef PFSS_SST_CATALOG_BOXED_HPP
#define PFSS_SST_CATALOG_BOXED_HPP

#include <pfss/sst/PFSS_SST_CPP_CONSTEXPR.hpp>
#include <utility>

namespace pfss { namespace sst {

template<class T, class Tag>
class boxed {
  T value_;

public:
  using value_type = T;

  // Construction is forwarded to T.
  template<class... Args>
  explicit constexpr boxed(Args &&... args)
      : value_(std::forward<Args>(args)...) {
  }

  // Copy construction, copy assignment, move construction, move
  // assignment, and destruction are intentionally implicit.

  //--------------------------------------------------------------------
  // Value retrieval
  //--------------------------------------------------------------------

  explicit PFSS_SST_CPP14_CONSTEXPR operator T &() noexcept {
    return value_;
  }

  explicit constexpr operator T const &() const noexcept {
    return value_;
  }

  PFSS_SST_CPP14_CONSTEXPR T & value() noexcept {
    return value_;
  }

  constexpr T const & value() const noexcept {
    return value_;
  }

  //--------------------------------------------------------------------
  // Comparisons for aliases and publicly derived classes
  //--------------------------------------------------------------------

  constexpr bool operator<(boxed const & b) const
      noexcept(noexcept(std::declval<T>() < std::declval<T>())) {
    return value() < b.value();
  }

  constexpr bool operator>(boxed const & b) const
      noexcept(noexcept(std::declval<T>() > std::declval<T>())) {
    return value() > b.value();
  }

  constexpr bool operator<=(boxed const & b) const
      noexcept(noexcept(std::declval<T>() <= std::declval<T>())) {
    return value() <= b.value();
  }

  constexpr bool operator>=(boxed const & b) const
      noexcept(noexcept(std::declval<T>() >= std::declval<T>())) {
    return value() >= b.value();
  }

  constexpr bool operator==(boxed const & b) const
      noexcept(noexcept(std::declval<T>() == std::declval<T>())) {
    return value() == b.value();
  }

  constexpr bool operator!=(boxed const & b) const
      noexcept(noexcept(std::declval<T>() != std::declval<T>())) {
    return value() != b.value();
  }

  //--------------------------------------------------------------------
  // Comparisons for privately derived classes
  //--------------------------------------------------------------------

  constexpr bool operator<(Tag const & b) const
      noexcept(noexcept(std::declval<T>() < std::declval<T>())) {
    return value() < b.value();
  }

  constexpr bool operator>(Tag const & b) const
      noexcept(noexcept(std::declval<T>() > std::declval<T>())) {
    return value() > b.value();
  }

  constexpr bool operator<=(Tag const & b) const
      noexcept(noexcept(std::declval<T>() <= std::declval<T>())) {
    return value() <= b.value();
  }

  constexpr bool operator>=(Tag const & b) const
      noexcept(noexcept(std::declval<T>() >= std::declval<T>())) {
    return value() >= b.value();
  }

  constexpr bool operator==(Tag const & b) const
      noexcept(noexcept(std::declval<T>() == std::declval<T>())) {
    return value() == b.value();
  }

  constexpr bool operator!=(Tag const & b) const
      noexcept(noexcept(std::declval<T>() != std::declval<T>())) {
    return value() != b.value();
  }
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_BOXED_HPP
