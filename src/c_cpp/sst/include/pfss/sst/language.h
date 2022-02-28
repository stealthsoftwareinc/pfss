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

#ifndef PFSS_SST_LANGUAGE_H
#define PFSS_SST_LANGUAGE_H

#include <pfss/sst/config.h>

/*----------------------------------------------------------------------
// C and C++
//--------------------------------------------------------------------*/

/*----------------------------------------------------------------------
// C only
//--------------------------------------------------------------------*/
#ifndef __cplusplus

#include <pfss/sst/PFSS_SST_C_OR_LATER.h>
#include <pfss/sst/PFSS_SST_C_VALUE.h>
#include <pfss/sst/catalog/PFSS_SST_STATIC_ASSERT.h>

/*----------------------------------------------------------------------
// C++ only
//--------------------------------------------------------------------*/
#elif PFSS_SST_WITH_BUILD_GROUP_CPP_AUTOTOOLS

#include <pfss/sst/catalog/PFSS_SST_CONSTEXPR_ASSERT.hpp>
#include <pfss/sst/PFSS_SST_CPP_CONSTEXPR.hpp>
#include <pfss/sst/catalog/PFSS_SST_CPP17_INLINE.hpp>
#include <pfss/sst/PFSS_SST_CPP_OR_LATER.hpp>
#include <pfss/sst/PFSS_SST_CPP_VALUE.hpp>
#include <pfss/sst/catalog/PFSS_SST_STATIC_ASSERT.hpp>

/*--------------------------------------------------------------------*/
#endif

#endif /* #ifndef PFSS_SST_LANGUAGE_H */
