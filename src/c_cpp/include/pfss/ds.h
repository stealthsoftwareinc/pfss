/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#ifndef PFSS_DS_H
#define PFSS_DS_H

// Include first to test independence.
#include <pfss.h>
// Include twice to test idempotence.
#include <pfss.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <stdint.h>
#include <type_traits>
#include <utility>

#include <pfss/common.h>

namespace pfss {

//
//
//
//
//
// Ds-2017-827
//
//
//
//
//

template<
    int DomainBits,
    int RangeBits,
    class DomainType,
    class RangeType,
    class BlockType,
    class RandPermType,
    class RngType>
class ds final {

public:
  static constexpr int domain_bits = DomainBits;
  static constexpr int range_bits = RangeBits;
  using domain_type = DomainType;
  using range_type = RangeType;
  using block_type = BlockType;
  using rand_perm_type = RandPermType;
  using rng_type = RngType;

  using nice_range_type = typename promote_unsigned<range_type>::type;
  // DEPRECATED
  using NiceRangeType = nice_range_type;

  static constexpr domain_type domain_mask =
      get_mask<domain_type>(domain_bits);
  static constexpr range_type range_mask =
      get_mask<range_type>(range_bits);

  static constexpr int domain_type_bits =
      std::numeric_limits<domain_type>::digits;
  static constexpr int range_type_bits =
      std::numeric_limits<range_type>::digits;

private:
  static_assert(DomainBits >= 1, "DomainBits must be at least 1");
  static_assert(
      std::is_unsigned<domain_type>::value,
      "domain_type must be an unsigned integer type");
  static_assert(
      !std::is_const<domain_type>::value,
      "domain_type must not be const");
  static_assert(
      !std::is_volatile<domain_type>::value,
      "domain_type must not be volatile");
  static_assert(
      !std::is_same<domain_type, bool>::value,
      "domain_type must not be bool");
  static_assert(
      domain_type_bits >= DomainBits,
      "domain_type must have at least DomainBits bits");

  static_assert(RangeBits >= 8, "RangeBits must be at least 8");
  static_assert(
      std::is_unsigned<range_type>::value,
      "range_type must be an unsigned integer type");
  static_assert(
      !std::is_const<range_type>::value,
      "range_type must not be const");
  static_assert(
      !std::is_volatile<range_type>::value,
      "range_type must not be volatile");
  static_assert(
      !std::is_same<range_type, bool>::value,
      "range_type must not be bool");
  static_assert(
      range_type_bits >= RangeBits,
      "range_type must have at least RangeBits bits");
  static_assert(
      RangeBits < block_type::bits,
      "RangeBits must be strictly less than block_type::bits");

  static constexpr bool a = 0;
  static constexpr bool b = 1;
  static constexpr bool L = 0;
  static constexpr bool R = 1;

  template<bool X>
  static block_type prg(rand_perm_type & pi, block_type x) {
    if (X == R) {
      x = x.flip_msb();
    }
    // Correlation robustness (GKWY-2019-074).
    return pi(x) ^ x;
  }

  static nice_range_type convert(block_type const block) {
    assert(range_bits > 0);
    int const q = range_bits / uchar_bits;
    int const r = range_bits % uchar_bits;
    int i = q + (r != 0);
    nice_range_type x = block[--i];
    if (r != 0) {
      x &= get_mask<nice_range_type>(r);
    }
    while (i != 0) {
      x <<= 8;
      x |= static_cast<nice_range_type>(block[--i]);
    }
    return x;
  }

public:
  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  struct cwthing {
    block_type scw;
    bool tcw[2];
  };

  struct key_type {
    int domain_bits;
    int range_bits;
    bool party;
    block_type s;
    cwthing cw[DomainBits];
    range_type cw_last;
  };

  // TODO:
  // For line 12, there's a question of whether using a lookup table
  // versus ternary expressions gives more performance.
  static void
  gen(key_type key[2],
      domain_type domain_slot,
      range_type range_value,
      rand_perm_type & pi,
      rng_type & rng) {
    key[0].domain_bits = DomainBits;
    key[1].domain_bits = DomainBits;
    key[0].range_bits = RangeBits;
    key[1].range_bits = RangeBits;
    auto const m = DomainBits;
    domain_type const alpha = domain_slot;
    nice_range_type const beta = range_value;
    key[a].party = a;
    key[b].party = b;
    // Line 2.
    block_type sprime[2];
    sprime[a] = rng();
    sprime[b] = rng();
    key[a].s = sprime[a];
    key[b].s = sprime[b];
    // Line 3.
    bool t[2] = {0, 1};
    // Line 4.
    for (decltype(+m) j = 0; j < m; ++j) {
      block_type sa[2], sb[2];
      // Line 5.
      sa[0] = prg<0>(pi, sprime[a]);
      sa[1] = prg<1>(pi, sprime[a]);
      sb[0] = prg<0>(pi, sprime[b]);
      sb[1] = prg<1>(pi, sprime[b]);
      // Line 6.
      bool const alpha_j = (alpha >> j) & 1;
      block_type const sigma_j = sa[!alpha_j] ^ sb[!alpha_j];
      // Lines 7-8.
      bool tau[2];
      tau[0] = sa[0].get_msb() ^ sb[0].get_msb() ^ alpha_j ^ 1;
      tau[1] = sa[1].get_msb() ^ sb[1].get_msb() ^ alpha_j;
      // Iteratively fill the correction words in the key
      for (int p = 0; p < 2; ++p) {
        key[p].cw[j].scw = sigma_j;
        key[p].cw[j].tcw[0] = tau[0];
        key[p].cw[j].tcw[1] = tau[1];
      }
      // Line 9.
      // Note that although t[a] and t[b] are opposites for the first loop
      // iteration, they're not necessarily opposites for the rest of the
      // loop iterations.
      // Note that the ePrint paper has a typo on line 9:
      //   \bar{\alpha}_j should be \alpha_j
      sprime[a] = t[a] ? sa[alpha_j] ^ sigma_j : sa[alpha_j];
      sprime[b] = t[b] ? sb[alpha_j] ^ sigma_j : sb[alpha_j];
      // Line 10.
      t[a] = sa[alpha_j].get_msb() ^ (t[a] & tau[alpha_j]);
      t[b] = sb[alpha_j].get_msb() ^ (t[b] & tau[alpha_j]);
    }
    // Line 11.
    key[0].cw_last = beta - convert(sprime[a]) + convert(sprime[b]);
    if (t[b]) {
      key[0].cw_last = -key[0].cw_last;
    }
    auto const G_mask = std::numeric_limits<range_type>::max() >>
        (std::numeric_limits<range_type>::digits - RangeBits);
    key[0].cw_last &= G_mask;
    key[1].cw_last = key[0].cw_last;
  }

  static range_type
  eval(key_type const & key, domain_type const x, rand_perm_type & pi) {
    block_type sprime = key.s;
    bool t = key.party;
    for (decltype(+DomainBits) j = 0; j != DomainBits; ++j) {
      bool const x_j = (x >> j) & 1;
      block_type sigma_j = key.cw[j].scw;
      block_type sprg;
      if (x_j == 0) {
        sprg = prg<0>(pi, sprime);
      } else {
        sprg = prg<1>(pi, sprime);
      }
      sprime = t ? sprg ^ sigma_j : sprg;
      t = t ? sprg.get_msb() ^ key.cw[j].tcw[x_j] : sprg.get_msb();
    }
    nice_range_type y = convert(sprime) +
        static_cast<nice_range_type>(t ? key.cw_last : 0);
    if (key.party == 1) {
      y = -y;
    }

    auto const G_mask = std::numeric_limits<range_type>::max() >>
        (std::numeric_limits<range_type>::digits - RangeBits);
    return static_cast<range_type>(y & G_mask);
  }

  template<class RangeTypeContainer>
  static void eval_all(
      key_type const & key,
      RangeTypeContainer & ys,
      rand_perm_type & pi) {
    auto const domain_max = std::numeric_limits<domain_type>::max() >>
        (std::numeric_limits<domain_type>::digits - DomainBits);
    domain_type x = 0;
    while (true) {
      ys[x] = eval(key, x, pi);
      if (x == domain_max) {
        break;
      }
      ++x;
    }
  }
};

} // namespace pfss

#endif // PFSS_DS_H
