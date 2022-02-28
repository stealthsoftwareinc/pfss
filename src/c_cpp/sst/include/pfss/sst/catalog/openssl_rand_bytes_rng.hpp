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

#ifndef PFSS_SST_CATALOG_OPENSSL_RAND_BYTES_RNG_HPP
#define PFSS_SST_CATALOG_OPENSSL_RAND_BYTES_RNG_HPP

#include <pfss/sst/catalog/openssl_rand_bytes_rng_core.hpp>
#include <pfss/sst/catalog/rng_shell.hpp>
#include <pfss/sst/config.h>

#if PFSS_SST_WITH_OPENSSL_CRYPTO

namespace pfss { namespace sst {

using openssl_rand_bytes_rng =
    ::pfss::sst::rng_shell<::pfss::sst::openssl_rand_bytes_rng_core>;

}} // namespace pfss::sst

#endif // #if PFSS_SST_WITH_OPENSSL_CRYPTO

#endif // #ifndef PFSS_SST_CATALOG_OPENSSL_RAND_BYTES_RNG_HPP
