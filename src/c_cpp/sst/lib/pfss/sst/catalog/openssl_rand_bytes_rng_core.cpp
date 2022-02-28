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
#include <pfss/sst/catalog/openssl_rand_bytes_rng_core.hpp>
// Include twice to test idempotence.
#include <pfss/sst/catalog/openssl_rand_bytes_rng_core.hpp>
//

#include <pfss/sst/catalog/PFSS_SST_ASSERT.hpp>
#include <pfss/sst/config.h>

#if PFSS_SST_WITH_OPENSSL_CRYPTO

#include <cstddef>
#include <openssl/rand.h>
#include <stdexcept>

namespace pfss { namespace sst {

openssl_rand_bytes_rng_core::openssl_rand_bytes_rng_core()
    : openssl_rand_bytes_rng_core(1024) {
}

std::size_t openssl_rand_bytes_rng_core::buffer_size() const noexcept {
  return buffer_size_;
}

void openssl_rand_bytes_rng_core::read(unsigned char * const dst,
                                       read_size_t const n) {
  PFSS_SST_ASSERT((n >= 0));
  if (n > 0) {
    int const s = RAND_bytes(dst, n);
    if (s != 1) {
      // TODO: Add more error information.
      throw std::runtime_error("RAND_bytes() failed");
    }
  }
}

}} // namespace pfss::sst

#endif // #if PFSS_SST_WITH_OPENSSL_CRYPTO
