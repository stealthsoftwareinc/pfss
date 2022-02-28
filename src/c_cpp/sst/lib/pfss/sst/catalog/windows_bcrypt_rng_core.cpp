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
#include <pfss/sst/catalog/windows_bcrypt_rng_core.hpp>
// Include twice to test idempotence.
#include <pfss/sst/catalog/windows_bcrypt_rng_core.hpp>
//

#include <pfss/sst/config.h>

#if PFSS_SST_WITH_WINDOWS_BCRYPT

#include <cstddef>
#include <stdexcept>

#include <windows.h>

#include <bcrypt.h>
#include <ntdef.h>

namespace pfss { namespace sst {

windows_bcrypt_rng_core::windows_bcrypt_rng_core()
    : windows_bcrypt_rng_core(1024) {
}

std::size_t windows_bcrypt_rng_core::buffer_size() const noexcept {
  return buffer_size_;
}

void windows_bcrypt_rng_core::read(unsigned char * const dst,
                                   read_size_t const n) {
  if (n > 0) {
    NTSTATUS const s = BCryptGenRandom(nullptr,
                                       dst,
                                       n,
                                       BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!NT_SUCCESS(s)) {
      // TODO: Add more error information. We should have a system for
      //       converting NTSTATUS values into error messages.
      throw std::runtime_error("BCryptGenRandom() failed");
    }
  }
}

}} // namespace pfss::sst

#endif // #if PFSS_SST_WITH_WINDOWS_BCRYPT
