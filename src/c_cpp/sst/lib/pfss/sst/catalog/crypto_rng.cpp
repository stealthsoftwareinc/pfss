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
#include <pfss/sst/catalog/crypto_rng.hpp>
// Include twice to test idempotence.
#include <pfss/sst/catalog/crypto_rng.hpp>
//

#include <pfss/sst/catalog/crypto_rng_t.hpp>

namespace pfss { namespace sst {

::pfss::sst::crypto_rng_t & crypto_rng() {
  static thread_local ::pfss::sst::crypto_rng_t rng;
  return rng;
}

}} // namespace pfss::sst
