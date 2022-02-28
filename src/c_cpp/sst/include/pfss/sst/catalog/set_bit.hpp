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

#ifndef PFSS_SST_CATALOG_SET_BIT_HPP
#define PFSS_SST_CATALOG_SET_BIT_HPP

#include <cassert>
#include <limits>
#include <pfss/sst/catalog/ones_mask.hpp>
#include <pfss/sst/catalog/type_msb.hpp>
#include <pfss/sst/catalog/uchar_msb.hpp>
#include <pfss/sst/integer.h>
#include <pfss/sst/language.h>
#include <pfss/sst/limits.h>
#include <type_traits>

namespace pfss { namespace sst {

template<
    class X,
    class Q,
    class R,
    typename std::enable_if<std::is_integral<X>::value, int>::type = 0>
constexpr X
set_bit(X const x, Q const q, R const r, bool const b) noexcept {
  PFSS_SST_STATIC_ASSERT(std::is_integral<X>::value);
  PFSS_SST_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_SST_STATIC_ASSERT(std::is_integral<R>::value);
  PFSS_SST_CONSTEXPR_ASSERT(!is_negative(q));
  PFSS_SST_CONSTEXPR_ASSERT(!is_negative(r));
  using UX = typename promote_unsigned<X>::type;
#define PFSS_SST_X_max (std::numeric_limits<X>::max())
#define PFSS_SST_X_is_signed (std::is_signed<X>::value)
#define PFSS_SST_X_width (std::numeric_limits<X>::digits + PFSS_SST_X_is_signed)
#define PFSS_SST_char_bit (std::numeric_limits<unsigned char>::digits)
  PFSS_SST_CONSTEXPR_ASSERT(unsigned_le(r, PFSS_SST_X_width - 1));
  PFSS_SST_CONSTEXPR_ASSERT(unsigned_le(
      q,
      (PFSS_SST_X_width - 1 - static_cast<int>(r)) / PFSS_SST_char_bit));
#define PFSS_SST_i (static_cast<int>(q) * PFSS_SST_char_bit + static_cast<int>(r))
#define PFSS_SST_mask (static_cast<UX>(1) << PFSS_SST_i)
  return PFSS_SST_X_is_signed && PFSS_SST_i == PFSS_SST_X_width - 1 ?
             (b == is_negative(x) ?
                  x :
                  b ? x - PFSS_SST_X_max - static_cast<X>(1) :
                      x + PFSS_SST_X_max + static_cast<X>(1)) :
             static_cast<X>((b ? static_cast<UX>(x) | PFSS_SST_mask :
                                 static_cast<UX>(x) & ~PFSS_SST_mask)
                            & static_cast<UX>(PFSS_SST_X_max))
                 - (is_negative(x) ? PFSS_SST_X_max : static_cast<X>(0))
                 - (is_negative(x) ? static_cast<X>(1) :
                                     static_cast<X>(0));
#undef PFSS_SST_mask
#undef PFSS_SST_i
#undef PFSS_SST_char_bit
#undef PFSS_SST_X_width
#undef PFSS_SST_X_is_signed
#undef PFSS_SST_X_max
}

template<
    class X,
    class R,
    typename std::enable_if<std::is_integral<X>::value, int>::type = 0>
constexpr X set_bit(X const x, R const r, bool const b) noexcept {
  return set_bit(x, 0, r, b);
}

template<class Q, class R>
void *
set_bit(void * const x, Q const q, R const r, bool const b) noexcept {
  PFSS_SST_STATIC_ASSERT(std::is_integral<Q>::value);
  PFSS_SST_STATIC_ASSERT(std::is_integral<R>::value);
  assert(x != nullptr);
  assert(!is_negative(q));
  assert(!is_negative(r));
  auto const rq = r / std::numeric_limits<unsigned char>::digits;
  auto const rr = r % std::numeric_limits<unsigned char>::digits;
  if (b) {
    *(static_cast<unsigned char *>(x) + q + rq) |=
        static_cast<unsigned char>(1U << rr);
  } else {
    *(static_cast<unsigned char *>(x) + q + rq) &=
        static_cast<unsigned char>(~(1U << rr));
  }
  return x;
}

template<class R>
void * set_bit(void * const x, R const r, bool const b) noexcept {
  return set_bit(x, 0, r, b);
}

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_SET_BIT_HPP
