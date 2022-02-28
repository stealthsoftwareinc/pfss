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
#include <pfss/sst/catalog/mono_time_zero.hpp>
// Include twice to test idempotence.
#include <pfss/sst/catalog/mono_time_zero.hpp>
//

#include <chrono>
#include <mutex>

namespace pfss { namespace sst {

namespace {

std::once_flag f;
std::chrono::time_point<std::chrono::steady_clock> z;

} // namespace

std::chrono::time_point<std::chrono::steady_clock> mono_time_zero() {
  std::call_once(f, [] { z = std::chrono::steady_clock::now(); });
  return z;
}

}} // namespace pfss::sst
