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

#ifndef PFSS_SST_CATALOG_FROM_BITS_HPP
#define PFSS_SST_CATALOG_FROM_BITS_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <pfss/sst/bit.h>
#include <pfss/sst/catalog/char_bit_v.hpp>
#include <pfss/sst/catalog/is_exact_width_integer.hpp>
#include <pfss/sst/catalog/is_little_endian.hpp>
#include <pfss/sst/catalog/is_negative.hpp>
#include <pfss/sst/catalog/promote_unsigned.hpp>
#include <pfss/sst/catalog/unsigned_ne.hpp>
#include <pfss/sst/catalog/width_bits.hpp>
#include <pfss/sst/integer.h>
#include <pfss/sst/language.h>
#include <type_traits>

namespace pfss { namespace sst {

template<
    class Y,
    bool SignExtend = std::is_signed<Y>::value,
    class Q,
    class R,
    class N,
    typename std::enable_if<std::is_integral<Y>::value, int>::type = 0>
Y from_bits(void const * const x,
            Q const q,
            R const r,
            N const n) noexcept {
  PFSS_SST_STATIC_ASSERT(std::is_integral<Y>::value);
  PFSS_SST_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_SST_STATIC_ASSERT(std::is_integral<R>::value);
  PFSS_SST_STATIC_ASSERT(std::is_integral<N>::value);

  assert(x != nullptr);
  assert(!::pfss::sst::is_negative(q));
  assert(!::pfss::sst::is_negative(r));
  assert(n > 0);

  auto const xp = reinterpret_cast<unsigned char const *>(x);
  auto const qu = ::pfss::sst::to_unsigned(q);
  auto const ru = ::pfss::sst::to_unsigned(r);
  auto const nu = ::pfss::sst::to_unsigned(n);
  constexpr auto cbu = ::pfss::sst::to_unsigned(::pfss::sst::char_bit_v);

  if (::pfss::sst::is_exact_width_integer<Y>::value
      && ::pfss::sst::is_little_endian<Y>::value && ru % cbu == 0
      && nu / cbu >= sizeof(Y)) {
    Y y;
    auto const src = xp + qu + ru / cbu;
    auto const end = src + sizeof(Y);
    auto const dst = reinterpret_cast<unsigned char *>(&y);
    std::copy(src, end, dst);
    return y;
  }

  using sz = typename ::pfss::sst::promote_unsigned<std::size_t>::type;
  using UR = typename ::pfss::sst::promote_unsigned<R>::type;
  Y y = 0;
  constexpr int Y_width = ::pfss::sst::width_bits<Y>::value;
  constexpr sz char_bit = std::numeric_limits<unsigned char>::digits;
  constexpr sz size_max = std::numeric_limits<std::size_t>::max();
  sz i = static_cast<sz>(q)
         + static_cast<sz>(static_cast<UR>(r) / char_bit);
  sz j = static_cast<sz>(static_cast<UR>(r) % char_bit);
  int k = 0;
  bool b = false;
  for (; k != Y_width && ::pfss::sst::unsigned_ne(k, n); ++k) {
    b = get_bit(x, i, j);
    y = set_bit(y, k, b);
    if (j == size_max) {
      i += j / char_bit;
      j %= char_bit;
    }
    ++j;
  }
  if (SignExtend && b) {
    for (; k != Y_width; ++k) {
      y = set_bit(y, k, b);
    }
  }
  return y;
}

template<
    class Y,
    bool SignExtend = std::is_signed<Y>::value,
    class R,
    class N,
    typename std::enable_if<std::is_integral<Y>::value, int>::type = 0>
Y from_bits(void const * const x, R const r, N const n) noexcept {
  return from_bits<Y, SignExtend>(x, 0, r, n);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_FROM_BITS_HPP
