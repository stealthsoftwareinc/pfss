/*
 * Copyright (C) 2012-2022 Stealth Software Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PFSS_SST_INTEGER_H
#define PFSS_SST_INTEGER_H

#include <pfss/sst/config.h>

/*----------------------------------------------------------------------
// C and C++
//--------------------------------------------------------------------*/

/*----------------------------------------------------------------------
// C only
//--------------------------------------------------------------------*/
#ifndef __cplusplus

/*----------------------------------------------------------------------
// C++ only
//--------------------------------------------------------------------*/
#elif PFSS_SST_WITH_BUILD_GROUP_CPP_AUTOTOOLS

#include <pfss/sst/catalog/can_represent_all.hpp>
#include <pfss/sst/catalog/is_negative.hpp>
#include <pfss/sst/catalog/perfect_eq.hpp>
#include <pfss/sst/catalog/perfect_ge.hpp>
#include <pfss/sst/catalog/perfect_gt.hpp>
#include <pfss/sst/catalog/perfect_le.hpp>
#include <pfss/sst/catalog/perfect_lt.hpp>
#include <pfss/sst/catalog/perfect_ne.hpp>
#include <pfss/sst/catalog/promote_unsigned.hpp>
#include <pfss/sst/catalog/promote_with_sign.hpp>
#include <pfss/sst/catalog/to_unsigned.hpp>
#include <pfss/sst/catalog/unsigned_eq.hpp>
#include <pfss/sst/catalog/unsigned_ge.hpp>
#include <pfss/sst/catalog/unsigned_gt.hpp>
#include <pfss/sst/catalog/unsigned_le.hpp>
#include <pfss/sst/catalog/unsigned_lt.hpp>
#include <pfss/sst/catalog/unsigned_ne.hpp>

/*--------------------------------------------------------------------*/
#endif

#endif /* #ifndef PFSS_SST_INTEGER_H */
