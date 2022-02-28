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

#ifndef PFSS_SST_PRIVATE_PFSS_SST_DLL_EXPORT_H
#define PFSS_SST_PRIVATE_PFSS_SST_DLL_EXPORT_H

#ifndef PFSS_SST_DLL_EXPORT

#if !defined(_WIN32) || !defined(DLL_EXPORT)
#define PFSS_SST_DLL_EXPORT
#elif defined(PFSS_SST_BUILD)
#define PFSS_SST_DLL_EXPORT __declspec(dllexport)
#else
#define PFSS_SST_DLL_EXPORT __declspec(dllimport)
#endif

#endif /* #ifndef PFSS_SST_DLL_EXPORT */

#endif /* #ifndef PFSS_SST_PRIVATE_PFSS_SST_DLL_EXPORT_H */
