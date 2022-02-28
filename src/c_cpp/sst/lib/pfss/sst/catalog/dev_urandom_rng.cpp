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
#include <pfss/sst/catalog/dev_urandom_rng.hpp>
// Include twice to test idempotence.
#include <pfss/sst/catalog/dev_urandom_rng.hpp>
//

#include <pfss/sst/config.h>

#if PFSS_SST_WITH_DEV_URANDOM

#include <pfss/sst/catalog/fixed_file_rng.hpp>

namespace pfss { namespace sst {

namespace guts {
namespace dev_urandom_rng {

char const file[] = "/dev/urandom";

} // namespace dev_urandom_rng
} // namespace guts

template class fixed_file_rng<::pfss::sst::guts::dev_urandom_rng::file>;

}} // namespace pfss::sst

#endif // #if PFSS_SST_WITH_DEV_URANDOM
