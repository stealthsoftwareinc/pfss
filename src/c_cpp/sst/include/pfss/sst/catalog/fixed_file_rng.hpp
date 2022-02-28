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

#ifndef PFSS_SST_CATALOG_FIXED_FILE_RNG_HPP
#define PFSS_SST_CATALOG_FIXED_FILE_RNG_HPP

#include <utility>

#include <pfss/sst/private/PFSS_SST_DLL_EXPORT.h>
#include <pfss/sst/catalog/PFSS_SST_STATIC_ASSERT.hpp>
#include <pfss/sst/catalog/file_rng.hpp>

namespace pfss { namespace sst {

template<char const * File>
class fixed_file_rng final {

  PFSS_SST_STATIC_ASSERT((File != nullptr));

  using g_t = ::pfss::sst::file_rng;
  g_t g_;

public:

  using result_type = g_t::result_type;

  static result_type constexpr max() noexcept {
    return g_t::max();
  }

  static result_type constexpr min() noexcept {
    return g_t::min();
  }

  explicit fixed_file_rng(bool const keep_open = true)
      : g_(File, keep_open) {
  }

  template<class Size>
  explicit fixed_file_rng(bool const keep_open, Size const buffer_size)
      : g_(File, keep_open, buffer_size) {
  }

  fixed_file_rng(fixed_file_rng const &) = delete;

  fixed_file_rng(fixed_file_rng &&) noexcept = default;

  fixed_file_rng & operator=(fixed_file_rng const &) = delete;

  fixed_file_rng & operator=(fixed_file_rng &&) = delete;

  ~fixed_file_rng() noexcept = default;

  result_type operator()() {
    return g_();
  }

  template<class... Args>
  auto operator()(Args &&... args)
      -> decltype(g_(std::forward<Args>(args)...)) {
    return g_(std::forward<Args>(args)...);
  }
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_FIXED_FILE_RNG_HPP
