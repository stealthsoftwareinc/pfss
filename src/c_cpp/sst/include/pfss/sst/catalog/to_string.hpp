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

#ifndef PFSS_SST_CATALOG_TO_STRING_HPP
#define PFSS_SST_CATALOG_TO_STRING_HPP

#include <string>

#include <pfss/sst/catalog/PFSS_SST_DEFINE_ALGORITHM.hpp>
#include <pfss/sst/catalog/decay_t.hpp>

namespace pfss { namespace sst {

PFSS_SST_DEFINE_ALGORITHM(to_string,
                     to_string_functor,
                     class Dst = std::string,
                     ::pfss::sst::decay_t<Dst>)

}} // namespace pfss::sst

#include <pfss/sst/private/to_string/from_char_input_iterable.hpp>
#include <pfss/sst/private/to_string/from_char_input_iterator.hpp>
#include <pfss/sst/private/to_string/from_floating.hpp>
#include <pfss/sst/private/to_string/from_integer.hpp>
#include <pfss/sst/private/to_string/from_member_function.hpp>
#include <pfss/sst/private/to_string/from_unscoped_enum.hpp>

#endif // #ifndef PFSS_SST_CATALOG_TO_STRING_HPP
