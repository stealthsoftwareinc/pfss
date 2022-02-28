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

// Include first to test independence.
#include <pfss/sst/catalog/file_rng_core.hpp>
// Include twice to test idempotence.
#include <pfss/sst/catalog/file_rng_core.hpp>
//

#include <cstdio>
#include <pfss/sst/catalog/PFSS_SST_ASSERT.hpp>
#include <stdexcept>
#include <string>
#include <utility>

namespace pfss { namespace sst {

void file_rng_core::open() {
  PFSS_SST_ASSERT((!moved_from_));
  if (stream_ == nullptr) {
    stream_ = std::fopen(file_.c_str(), "rb");
    if (stream_ == nullptr) {
      throw std::runtime_error("Error opening file: " + file_);
    }
  }
}

void file_rng_core::close() noexcept {
  PFSS_SST_ASSERT((!moved_from_));
  if (stream_ != nullptr) {
    static_cast<void>(std::fclose(stream_));
    stream_ = nullptr;
  }
}

file_rng_core::file_rng_core(std::string file, bool const keep_open)
    : file_rng_core(std::move(file), keep_open, 1024) {
}

file_rng_core::~file_rng_core() noexcept {
  if (!moved_from_) {
    close();
  }
}

std::size_t file_rng_core::buffer_size() const noexcept {
  return buffer_size_;
}

void file_rng_core::read(unsigned char * const dst,
                         std::size_t const n) {
  PFSS_SST_ASSERT((!moved_from_));
  if (n > 0) {
    open();
    if (std::fread(dst, n, 1, stream_) != 1) {
      throw std::runtime_error("Error reading file: " + file_);
    }
    if (!keep_open_) {
      close();
    }
  }
}

}} // namespace pfss::sst
