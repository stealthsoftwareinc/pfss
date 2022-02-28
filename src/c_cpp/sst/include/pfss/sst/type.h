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

#ifndef PFSS_SST_TYPE_H
#define PFSS_SST_TYPE_H

/*======================================================================
// C and C++
//====================================================================*/

#include <pfss/sst/config.h>

/*======================================================================
// C only
//====================================================================*/
#ifndef __cplusplus

/*======================================================================
// C++ only
//====================================================================*/
#elif PFSS_SST_WITH_BUILD_GROUP_CPP_AUTOTOOLS

#include <pfss/sst/catalog/is_bool.hpp>
#include <pfss/sst/catalog/is_non_bool_integer.hpp>
#include <pfss/sst/catalog/is_ones_complement.hpp>
#include <pfss/sst/catalog/is_sign_magnitude.hpp>
#include <pfss/sst/catalog/is_signed_integer.hpp>
#include <pfss/sst/catalog/is_twos_complement.hpp>
#include <pfss/sst/catalog/is_unsigned_integer.hpp>
#include <pfss/sst/catalog/promote.hpp>
#include <pfss/sst/catalog/promotes.hpp>
#include <pfss/sst/catalog/remove_cvref.hpp>
#include <pfss/sst/catalog/remove_cvref_t.hpp>

/*====================================================================*/
#endif

#endif /* #ifndef PFSS_SST_TYPE_H */
