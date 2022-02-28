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

#ifndef PFSS_SST_CATALOG_CRYPTO_RNG_T_HPP
#define PFSS_SST_CATALOG_CRYPTO_RNG_T_HPP

#include <pfss/sst/config.h>

#if 0

//----------------------------------------------------------------------
#elif PFSS_SST_WITH_WINDOWS_BCRYPT
//----------------------------------------------------------------------

#include <pfss/sst/catalog/windows_bcrypt_rng.hpp>

namespace pfss { namespace sst {

using crypto_rng_t = ::pfss::sst::windows_bcrypt_rng;

}} // namespace pfss::sst

//----------------------------------------------------------------------
#elif PFSS_SST_WITH_OPENSSL_CRYPTO
//----------------------------------------------------------------------

#include <pfss/sst/catalog/openssl_rand_bytes_rng.hpp>

namespace pfss { namespace sst {

using crypto_rng_t = ::pfss::sst::openssl_rand_bytes_rng;

}} // namespace pfss::sst

//----------------------------------------------------------------------
#elif PFSS_SST_WITH_DEV_URANDOM
//----------------------------------------------------------------------

#include <pfss/sst/catalog/dev_urandom_rng.hpp>

namespace pfss { namespace sst {

using crypto_rng_t = ::pfss::sst::dev_urandom_rng;

}} // namespace pfss::sst

//----------------------------------------------------------------------
#endif

#endif // #ifndef PFSS_SST_CATALOG_CRYPTO_RNG_T_HPP
