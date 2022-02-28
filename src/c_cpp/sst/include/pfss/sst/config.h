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

#ifndef PFSS_SST_CONFIG_H
#define PFSS_SST_CONFIG_H

#include <pfss/config.h>

/* TODO: This header will eventually disappear once all of the macros
 *       are moved into their own files. */

#include <pfss/sst/catalog/PFSS_SST_WITH_OPENSSL_CRYPTO.h>

/*----------------------------------------------------------------------
// Literal booleans
//----------------------------------------------------------------------
//
// These macros are always set to literal 1 or 0.
*/

#define PFSS_SST_HAVE_CYGWIN PFSS_HAVE_CYGWIN
#define PFSS_SST_HAVE_DEV_URANDOM PFSS_HAVE_DEV_URANDOM
#define PFSS_SST_HAVE_INT16_T PFSS_HAVE_INT16_T
#define PFSS_SST_HAVE_INT32_T PFSS_HAVE_INT32_T
#define PFSS_SST_HAVE_INT64_T PFSS_HAVE_INT64_T
#define PFSS_SST_HAVE_INT8_T PFSS_HAVE_INT8_T
#define PFSS_SST_HAVE_INTPTR_T PFSS_HAVE_INTPTR_T
#define PFSS_SST_HAVE_MINGW PFSS_HAVE_MINGW
#define PFSS_SST_HAVE_UINT16_T PFSS_HAVE_UINT16_T
#define PFSS_SST_HAVE_UINT32_T PFSS_HAVE_UINT32_T
#define PFSS_SST_HAVE_UINT64_T PFSS_HAVE_UINT64_T
#define PFSS_SST_HAVE_UINT8_T PFSS_HAVE_UINT8_T
#define PFSS_SST_HAVE_UINTPTR_T PFSS_HAVE_UINTPTR_T
#define PFSS_SST_HAVE_X86_32 PFSS_HAVE_X86_32
#define PFSS_SST_HAVE_X86_64 PFSS_HAVE_X86_64
#define PFSS_SST_WITH_AES_NI PFSS_WITH_AES_NI
#define PFSS_SST_WITH_BUILD_GROUP_CPP_AUTOTOOLS PFSS_WITH_BUILD_GROUP_CPP_AUTOTOOLS
#define PFSS_SST_WITH_CYGWIN PFSS_WITH_CYGWIN
#define PFSS_SST_WITH_DEV_URANDOM PFSS_WITH_DEV_URANDOM
#define PFSS_SST_WITH_GMP PFSS_WITH_GMP
#define PFSS_SST_WITH_JNI PFSS_WITH_JNI
#define PFSS_SST_WITH_MINGW PFSS_WITH_MINGW
#define PFSS_SST_WITH_NETTLE PFSS_WITH_NETTLE
#define PFSS_SST_WITH_NTL PFSS_WITH_NTL
#define PFSS_SST_WITH_OPENSSL_SSL PFSS_WITH_OPENSSL_SSL
#define PFSS_SST_WITH_POSIX PFSS_WITH_POSIX
#define PFSS_SST_WITH_WINDOWS PFSS_WITH_WINDOWS
#define PFSS_SST_WITH_WINDOWS_BCRYPT PFSS_WITH_WINDOWS_BCRYPT
#define PFSS_SST_WITH_WINDOWS_KERNEL32 PFSS_WITH_WINDOWS_KERNEL32
#define PFSS_SST_WITH_WINDOWS_WS2_32 PFSS_WITH_WINDOWS_WS2_32
#define PFSS_SST_WITH_X86 PFSS_WITH_X86
#define PFSS_SST_WITH_X86_32 PFSS_WITH_X86_32
#define PFSS_SST_WITH_X86_64 PFSS_WITH_X86_64

/*--------------------------------------------------------------------*/

#endif /* #ifndef PFSS_SST_CONFIG_H */
