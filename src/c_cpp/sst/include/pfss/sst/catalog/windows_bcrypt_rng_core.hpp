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

#ifndef PFSS_SST_CATALOG_WINDOWS_BCRYPT_RNG_CORE_HPP
#define PFSS_SST_CATALOG_WINDOWS_BCRYPT_RNG_CORE_HPP

#include <pfss/sst/catalog/PFSS_SST_ASSERT.hpp>
#include <pfss/sst/private/PFSS_SST_DLL_EXPORT.h>
#include <pfss/sst/catalog/moved_from.hpp>
#include <pfss/sst/catalog/size_max_v.hpp>
#include <pfss/sst/catalog/unsigned_lt.hpp>
#include <pfss/sst/config.h>

#if PFSS_SST_WITH_WINDOWS_BCRYPT

#include <cstddef>

namespace pfss { namespace sst {

class PFSS_SST_DLL_EXPORT windows_bcrypt_rng_core {
  ::pfss::sst::moved_from moved_from_;
  std::size_t buffer_size_;

protected:
  using read_size_t = unsigned long;

  template<class Size>
  explicit windows_bcrypt_rng_core(Size const buffer_size)
      : buffer_size_((PFSS_SST_ASSERT((buffer_size > 0)),
                      ::pfss::sst::unsigned_lt(buffer_size, ::pfss::sst::size_max_v) ?
                          static_cast<std::size_t>(buffer_size) :
                          static_cast<std::size_t>(::pfss::sst::size_max_v))) {
  }

  explicit windows_bcrypt_rng_core();

  windows_bcrypt_rng_core(windows_bcrypt_rng_core const &) = delete;

  windows_bcrypt_rng_core(windows_bcrypt_rng_core &&) noexcept =
      default;

  windows_bcrypt_rng_core &
  operator=(windows_bcrypt_rng_core const &) = delete;

  windows_bcrypt_rng_core &
  operator=(windows_bcrypt_rng_core &&) = delete;

  ~windows_bcrypt_rng_core() noexcept = default;

  std::size_t buffer_size() const noexcept;

  void read(unsigned char * dst, read_size_t n);
};

}} // namespace pfss::sst

#endif // #if PFSS_SST_WITH_WINDOWS_BCRYPT

#endif // #ifndef PFSS_SST_CATALOG_WINDOWS_BCRYPT_RNG_CORE_HPP
