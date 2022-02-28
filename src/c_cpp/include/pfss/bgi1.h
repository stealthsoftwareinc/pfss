//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BGI1_H
#define PFSS_BGI1_H

//
// This file implements Figure 1 from <https://ia.cr/2018/707>. For
// reference, here is a complete citation:
//
//       @misc{cryptoeprint:2018:707,
//           author = {Elette Boyle and Niv Gilboa and Yuval Ishai},
//           title = {Function Secret Sharing: Improvements and Extensions},
//           howpublished = {Cryptology ePrint Archive, Report 2018/707},
//           year = {2018},
//           note = {\url{https://eprint.iacr.org/2018/707}},
//       }
//
// TODO: serialize/deserialize range type containers -- perhaps add those as
// member functions of a z2n group class. Modify the convert portions of gen
// and eval. Write unit test.

// Include first to test independence.
#include <pfss.h>
// Include twice to test idempotence.
#include <pfss.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>

#include <pfss/common.h>

namespace pfss {

template<class DomainType,
         class RangeType,
         class BlockType,
         class RandPermType,
         class RngType,
         class RangePack,
         class PrgConvert>
struct bgi1_common {

  PFSS_SST_STATIC_ASSERT(
      (std::is_same<typename remove_cvref<RangePack>::type,
                    std::true_type>::value
       || std::is_same<typename remove_cvref<RangePack>::type,
                       std::false_type>::value));

  PFSS_SST_STATIC_ASSERT(
      (std::is_same<typename remove_cvref<PrgConvert>::type,
                    std::true_type>::value
       || std::is_same<typename remove_cvref<PrgConvert>::type,
                       std::false_type>::value));

  using domain_type = typename remove_cvref<DomainType>::type;
  using range_type = typename remove_cvref<RangeType>::type;
  using block_type = typename remove_cvref<BlockType>::type;
  using rand_perm_type = typename remove_cvref<RandPermType>::type;
  using rng_type = typename remove_cvref<RngType>::type;
  static constexpr bool range_pack =
      remove_cvref<RangePack>::type::value;
  static constexpr bool prg_convert =
      remove_cvref<PrgConvert>::type::value;

  PFSS_SST_STATIC_ASSERT(is_unsigned_integer<range_type>::value);

  static constexpr int range_type_bits = type_bits<range_type>();
  using nice_range_type = typename promote_unsigned<range_type>::type;
  static constexpr int block_bits = block_type::bits;
  static constexpr int cw_last_bits =
      prg_convert ? block_bits * 2 : block_bits - 1;

  static nice_range_type convert(int const range_bits,
                                 int const n_minus_v,
                                 void const * const blocks,
                                 int const index) noexcept {
    assert(blocks != nullptr);
    assert(index >= 0);
    assert(index < (1 << n_minus_v));
    if (range_bits % uchar_bits != 0) {
      return from_bits<nice_range_type>(blocks,
                                        0,
                                        index * range_bits,
                                        range_bits);
    } else if (system_is_little_endian()
               && sizeof(range_type) == range_bits / uchar_bits) {
      range_type x;
      std::memcpy(&x,
                  reinterpret_cast<uint8_t const *>(blocks)
                      + index * sizeof(range_type),
                  sizeof(range_type));
      return x;
    } else {
      auto const n = range_bits / uchar_bits;
      range_type x = 0;
      uint8_t const * p =
          reinterpret_cast<uint8_t const *>(blocks) + index * n;
      for (decltype(+n) i = 0; i != n; ++i) {
        x |= static_cast<range_type>(*p++) << (i * uchar_bits);
      }
      return x;
    }
  }

  static nice_range_type convert(int const range_bits,
                                 int const n_minus_v,
                                 block_type const & block,
                                 int const index) noexcept {
    return convert(range_bits, n_minus_v, &block.raw(), index);
  }

  static nice_range_type convert(int const range_bits,
                                 int const n_minus_v,
                                 block_type const & block0,
                                 block_type const & block1,
                                 int const index) noexcept {
    typename block_type::raw_type const & raw0 = block0.raw();
    typename block_type::raw_type const & raw1 = block1.raw();
    // We're forced to use memcpy here because raw_type is sometimes an
    // array type, which breaks raws[2] = {raw0, raw1}.
    typename block_type::raw_type raws[2];
    std::memcpy(&raws[0], &raw0, sizeof(raw0));
    std::memcpy(&raws[1], &raw1, sizeof(raw1));
    return convert(range_bits, n_minus_v, &raws, index);
  }

  struct cw_type {
    block_type scw;
    std::array<bool, 2> tcw;
  };

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {0};
  }

  static constexpr std::size_t key_blob_size(int,
                                             int const range_bits,
                                             int const n_minus_v,
                                             int const v) noexcept {
    using T = promote_unsigned<std::size_t>::type;
    // clang-format off
    return static_cast<std::size_t>(T(0)

      // The header.
      + T(key_header().size())

      // The domain_bits value.
      + T(1)

      // The range_bits value.
      + T(1)

      // The s block.
      + T(block_type().size())

      // The scw blocks.
      + T(v) * T(block_type().size())

      // The party bit and the tcw bits.
      + T(1)
      + T(2) * T(bits_to_bytes(v))

      // The cw_last values.
      + T(bits_to_bytes(range_bits * (1 << n_minus_v)))

    );
    // clang-format on
  }

  template<class CwTypeContainer,
           class RangeTypeContainer,
           class ContiguousByteOutputIt>
  static ContiguousByteOutputIt
  serialize_key(int const domain_bits,
                int const range_bits,
                bool const party,
                block_type const & s,
                CwTypeContainer const & cw,
                RangeTypeContainer const & cw_last,
                ContiguousByteOutputIt out) noexcept {
    auto const h = key_header();
    out = std::copy(h.cbegin(), h.cend(), out);
    out = serialize_integer(uchar_bits, domain_bits, out);
    out = serialize_integer(uchar_bits, range_bits, out);
    out = s.serialize(out);
    for (auto const & x : cw) {
      out = x.scw.serialize(out);
    }
    *out++ = party;
    for (int j = 0; j != 2; ++j) {
      unsigned char d = 0;
      int i = 0;
      for (decltype(cw.size()) k = 0; k != cw.size(); ++k) {
        d |= static_cast<unsigned char>(cw[cw.size() - 1 - k].tcw[j])
             << i;
        if (++i == uchar_bits) {
          *out++ = d;
          d = 0;
          i = 0;
        }
      }
      if (i != 0) {
        *out++ = d;
      }
    }
    if (range_bits % uchar_bits != 0) {
      int i = 0;
      for (range_type const & x : cw_last) {
        to_bits(std::addressof(*out), i, x, range_bits);
        i += range_bits;
      }
      out += bits_to_bytes(i);
    } else if (system_is_little_endian()
               && sizeof(range_type) == range_bits / uchar_bits) {
      std::memcpy(std::addressof(*out),
                  cw_last.data(),
                  cw_last.size() * sizeof(range_type));
      out += cw_last.size() * sizeof(range_type);
    } else {
      auto const n = range_bits / uchar_bits;
      for (range_type const & x : cw_last) {
        for (decltype(+n) i = 0; i != n; ++i) {
          *out++ = static_cast<unsigned char>(x >> (i * uchar_bits));
        }
      }
    }
    return out;
  }

  // Assumes that header, domain_bits, and range_bits have already
  // been parsed and verified, and that the blob size has also been
  // verified.
  template<class CwTypeContainer,
           class RangeTypeContainer,
           class ContiguousByteInputIt>
  static ContiguousByteInputIt
  deserialize_key(int const domain_bits,
                  int const range_bits,
                  bool & party,
                  block_type & s,
                  CwTypeContainer & cw,
                  RangeTypeContainer & cw_last,
                  ContiguousByteInputIt in) {

    std::advance(in, key_header().size());

    unsigned char const db = *in++;
    if (unsigned_ne(db, domain_bits)) {
      throw std::invalid_argument("key blob is corrupt");
    }

    unsigned char const rb = *in++;
    if (unsigned_ne(rb, range_bits)) {
      throw std::invalid_argument("key blob is corrupt");
    }

    in = s.parse(in);

    for (cw_type & x : cw) {
      in = x.scw.parse(in);
    }

    unsigned char const pt = *in++;
    if (pt > 1) {
      throw std::invalid_argument("key blob is corrupt");
    }
    party = static_cast<bool>(pt);
    for (int j = 0; j != 2; ++j) {
      int i = 0;
      for (decltype(cw.size()) k = 0; k != cw.size(); ++k) {
        cw[cw.size() - 1 - k].tcw[j] = (*in >> i) & 1;
        if (++i == uchar_bits) {
          ++in;
          i = 0;
        }
      }
      if (i != 0) {
        ++in;
      }
    }

    if (range_bits % uchar_bits != 0) {
      int i = 0;
      for (range_type & x : cw_last) {
        x = from_bits<range_type>(std::addressof(*in), i, range_bits);
        i += range_bits;
      }
      in += bits_to_bytes(i);
    } else if (system_is_little_endian()
               && sizeof(range_type) == range_bits / uchar_bits) {
      std::memcpy(cw_last.data(),
                  std::addressof(*in),
                  cw_last.size() * sizeof(range_type));
      in += cw_last.size() * sizeof(range_type);
    } else {
      auto const n = range_bits / uchar_bits;
      for (range_type & x : cw_last) {
        x = 0;
        for (decltype(+n) i = 0; i != n; ++i) {
          x |= static_cast<range_type>(*in++) << (i * uchar_bits);
        }
      }
    }

    return in;
  }

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

  static block_type zero_msb(block_type x) {
    x = x.set_msb(0);
    return x;
  }

  // TODO:
  // For line 12, there's a question of whether using a lookup table
  // versus ternary expressions gives more performance.
  template<class KeyType>
  static void gen(int const domain_bits,
                  int const range_bits,
                  int const n_minus_v,
                  int const v,
                  KeyType key[2],
                  domain_type const & alpha,
                  range_type const beta,
                  rand_perm_type & pi,
                  rng_type & rng) noexcept {
    assert(key[0].domain_bits == domain_bits);
    assert(key[1].domain_bits == domain_bits);
    assert(key[0].range_bits == range_bits);
    assert(key[1].range_bits == range_bits);
    key[0].party = 0;
    key[1].party = 1;
    // Line 2.
    block_type s[2];
    s[0] = zero_msb(rng());
    s[1] = zero_msb(rng());
    key[0].s = s[0];
    key[1].s = s[1];
    // Line 3.
    bool t[2] = {0, 1};
    // Line 4.
    for (int i = 0; i != v; ++i) {
      block_type s0[2], s1[2];
      bool t0[2], t1[2];
      s0[L] = prg<L>(pi, s[0]);
      s0[R] = prg<R>(pi, s[0]);
      s1[L] = prg<L>(pi, s[1]);
      s1[R] = prg<R>(pi, s[1]);
      t0[L] = s0[L].get_msb();
      t0[R] = s0[R].get_msb();
      t1[L] = s1[L].get_msb();
      t1[R] = s1[R].get_msb();
      s0[L] = s0[L].set_msb(0);
      s0[R] = s0[R].set_msb(0);
      s1[L] = s1[L].set_msb(0);
      s1[R] = s1[R].set_msb(0);
      bool const alpha_i = getbitx(alpha, domain_bits - 1 - i);
      bool const keep = alpha_i;
      bool const lose = !alpha_i;
      block_type const scw = s0[lose] ^ s1[lose];
      bool tcw[2];
      tcw[L] = t0[L] ^ t1[L] ^ alpha_i ^ 1;
      tcw[R] = t0[R] ^ t1[R] ^ alpha_i;
      // Line 11.
      for (int j = 0; j < 2; ++j) {
        key[j].cw[i].scw = scw;
        key[j].cw[i].tcw[L] = tcw[L];
        key[j].cw[i].tcw[R] = tcw[R];
      }
      // Line 12.
      // Note that although t[0] and t[1] are opposites for the first loop
      // iteration, they're not necessarily opposites for the rest of the
      // loop iterations.
      s[0] = t[0] ? s0[keep] ^ scw : s0[keep];
      s[1] = t[1] ? s1[keep] ^ scw : s1[keep];
      // Line 13.
      t[0] = t0[keep] ^ (t[0] & tcw[keep]);
      t[1] = t1[keep] ^ (t[1] & tcw[keep]);
      // Line 14. (End of loop).
    }
    // Line 15.
    int cw_last_i = 0;
    for (int i = v; i != domain_bits; ++i) {
      cw_last_i <<= 1;
      cw_last_i |= getbitx(alpha, domain_bits - 1 - i);
    }
    block_type prg_L_s0;
    block_type prg_R_s0;
    block_type prg_L_s1;
    block_type prg_R_s1;
    if (prg_convert) {
      prg_L_s0 = prg<L>(pi, s[0]);
      prg_R_s0 = prg<R>(pi, s[0]);
      prg_L_s1 = prg<L>(pi, s[1]);
      prg_R_s1 = prg<R>(pi, s[1]);
    }
    for (int i = 0; i != (1 << n_minus_v); ++i) {
      if (prg_convert) {
        key[0].cw_last[i] =
            static_cast<nice_range_type>(
                i == cw_last_i ? beta : static_cast<range_type>(0))
            - convert(range_bits, n_minus_v, prg_L_s0, prg_R_s0, i)
            + convert(range_bits, n_minus_v, prg_L_s1, prg_R_s1, i);
      } else {
        key[0].cw_last[i] =
            static_cast<nice_range_type>(
                i == cw_last_i ? beta : static_cast<range_type>(0))
            - convert(range_bits, n_minus_v, s[0], i)
            + convert(range_bits, n_minus_v, s[1], i);
      }
      if (t[1]) {
        key[0].cw_last[i] = -key[0].cw_last[i];
      }
      key[0].cw_last[i] &= get_mask<range_type>(range_bits);
      key[1].cw_last[i] = key[0].cw_last[i];
    }
    // Line 16 doesn't happen because we already constructed the keys on
    // Lines 2, 11, and 15.
  }

  template<class KeyType>
  static range_type eval(int const domain_bits,
                         int const range_bits,
                         int const n_minus_v,
                         int const v,
                         KeyType const & key,
                         domain_type const & x,
                         rand_perm_type & pi) noexcept {
    block_type s = key.s;
    bool t = key.party;
    for (int i = 0; i != v; ++i) {
      bool const x_i = getbitx(x, domain_bits - 1 - i);
      block_type scw = key.cw[i].scw;
      if (x_i == 0) {
        s = prg<L>(pi, s);
      } else {
        s = prg<R>(pi, s);
      }
      if (t) {
        scw = scw.set_msb(key.cw[i].tcw[x_i]);
        s ^= scw;
      }
      t = s.get_msb();
      s = s.set_msb(0);
    }
    int cw_last_i = 0;
    for (int i = v; i != domain_bits; ++i) {
      cw_last_i <<= 1;
      cw_last_i |= getbitx(x, domain_bits - 1 - i);
    }
    nice_range_type y;
    if (prg_convert) {
      y = convert(range_bits,
                  n_minus_v,
                  prg<L>(pi, s),
                  prg<R>(pi, s),
                  cw_last_i)
          + static_cast<nice_range_type>(t ? key.cw_last[cw_last_i] :
                                             0);
    } else {
      y = convert(range_bits, n_minus_v, s, cw_last_i)
          + static_cast<nice_range_type>(t ? key.cw_last[cw_last_i] :
                                             0);
    }
    if (key.party == 1) {
      y = -y;
    }
    return static_cast<range_type>(
        y & get_mask<nice_range_type>(range_bits));
  }

  struct cache_entry_t {
    block_type s;
    bool t;
  };

  template<class KeyType, class CacheEntryArray, class OutputIt>
  static OutputIt pack_eval(int const domain_bits,
                            int const range_bits,
                            int const n_minus_v,
                            int const v,
                            int const pack_count,
                            KeyType const & key,
                            CacheEntryArray & cache,
                            int const b,
                            domain_type const & x,
                            rand_perm_type & pi,
                            OutputIt out) noexcept {
    block_type s;
    bool t;
    if (b == 0) {
      s = key.s;
      t = key.party;
    } else {
      s = cache[b - 1].s;
      t = cache[b - 1].t;
    }
    for (int i = b; i != v; ++i) {
      bool const x_i = getbitx(x, domain_bits - 1 - i);
      block_type scw = key.cw[i].scw;
      if (x_i == 0) {
        s = prg<L>(pi, s);
      } else {
        s = prg<R>(pi, s);
      }
      if (t) {
        scw = scw.set_msb(key.cw[i].tcw[x_i]);
        s ^= scw;
      }
      t = s.get_msb();
      s = s.set_msb(0);
      cache[i].s = s;
      cache[i].t = t;
    }
    block_type prg_a;
    block_type prg_b;
    if (prg_convert) {
      prg_a = prg<L>(pi, s);
      prg_b = prg<R>(pi, s);
    }
    for (int i = 0; i != pack_count; ++i) {
      nice_range_type y;
      if (prg_convert) {
        y = convert(range_bits, n_minus_v, prg_a, prg_b, i);
      } else {
        y = convert(range_bits, n_minus_v, s, i);
      }
      if (t == 1) {
        y += static_cast<nice_range_type>(key.cw_last[i]);
      }
      if (key.party == 1) {
        y = -y;
      }
      *out++ = static_cast<range_type>(
          y & get_mask<nice_range_type>(range_bits));
    }
    return out;
  }

#if 0
  template<class KeyType, class OutputIt>
  static OutputIt pack_eval_x4(int const domain_bits,
                               int const range_bits,
                               int const n_minus_v,
                               int const v,
                               int const pack_count,
                               KeyType const & key,
                               domain_type const & x0,
                               domain_type const & x1,
                               domain_type const & x2,
                               domain_type const & x3,
                               rand_perm_type & pi,
                               OutputIt out) noexcept {
    for (auto const & x :
         {std::cref(x0), std::cref(x1), std::cref(x2), std::cref(x3)}) {
      out = pack_eval(domain_bits,
                      range_bits,
                      n_minus_v,
                      v,
                      pack_count,
                      key,
                      x.get(),
                      pi,
                      out);
    }
    return out;
  }
#endif
};

template<int DomainBits,
         int RangeBits,
         class DomainType,
         class RangeType,
         class BlockType = default_block_t<>,
         class RandPermType = default_rand_perm_t<BlockType>,
         class RngType = default_rng_t<BlockType>,
         class RangePack = std::true_type,
         class PrgConvert = std::true_type>
class bgi1 final {

  using common = bgi1_common<DomainType,
                             RangeType,
                             BlockType,
                             RandPermType,
                             RngType,
                             RangePack,
                             PrgConvert>;

public:
  static constexpr int domain_bits = DomainBits;
  static constexpr int range_bits = RangeBits;
  using domain_type = typename common::domain_type;
  using range_type = typename common::range_type;
  using block_type = typename common::block_type;
  using rand_perm_type = typename common::rand_perm_type;
  using rng_type = typename common::rng_type;
  static constexpr bool range_pack = common::range_pack;
  static constexpr bool prg_convert = common::prg_convert;

  static constexpr int range_type_bits = common::range_type_bits;
  using nice_range_type = typename common::nice_range_type;
  static constexpr int block_bits = common::block_bits;
  static constexpr int cw_last_bits = common::cw_last_bits;

  // DEPRECATED
  using NiceRangeType = nice_range_type;

  PFSS_SST_STATIC_ASSERT(domain_bits > 0);
  PFSS_SST_STATIC_ASSERT(range_bits > 0);
  PFSS_SST_STATIC_ASSERT(range_bits <= range_type_bits);

  static constexpr range_type range_mask =
      get_mask<range_type>(range_bits);

  //
  // 1 << n_minus_v is the number of range elements that we can extract
  // from a cw_last blob. This is the number of range elements that fit
  // into cw_last_bits bits, rounded down to a power of two, clamped to
  // [0, 2^domain_bits]. The "n" in "n_minus_v" is just the name used
  // for domain_bits in the papers. In summary, 0 <= v <= domain_bits,
  // and v is the depth of the GGM tree.
  //

  static constexpr int n_minus_v = min(
      domain_bits,
      range_pack ? power_of_two_width(cw_last_bits / range_bits) : 0);

  static constexpr int v = domain_bits - n_minus_v;

  static constexpr int pack_count = 1 << n_minus_v;
  static constexpr int pack_mask = pack_count - 1;

  //
  // Ensure that at least one range element fits into a cw_last blob.
  //

  PFSS_SST_STATIC_ASSERT(range_bits <= cw_last_bits);

private:
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

  static block_type zero_msb(block_type x) {
    x = x.set_msb(0);
    return x;
  }

public:
  bgi1() noexcept = default;
  bgi1(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bgi1() noexcept = default;
  bgi1(bgi1 const &) noexcept = default;
  bgi1(bgi1 &&) noexcept = default;
  bgi1 & operator=(bgi1 const &) noexcept = default;
  bgi1 & operator=(bgi1 &&) noexcept = default;

  static std::array<unsigned char, 16> const & fixed_key() {
    return fixed_aes_128_key;
  }

  static constexpr std::size_t key_blob_size() noexcept {
    return key_type::size();
  }
  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  using cw_type = typename common::cw_type;

  struct key_type final {

    static constexpr int domain_bits = DomainBits;
    static constexpr int range_bits = RangeBits;
    bool party;
    block_type s;
    std::array<cw_type, v> cw;
    PFSS_SST_STATIC_ASSERT(unsigned_le(1 << n_minus_v, size_max::value));
    std::array<range_type, 1 << n_minus_v> cw_last;

    static constexpr std::size_t size() noexcept {
      return common::key_blob_size(DomainBits, RangeBits, n_minus_v, v);
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      return common::serialize_key(DomainBits,
                                   RangeBits,
                                   party,
                                   s,
                                   cw,
                                   cw_last,
                                   out);
    }

    // Assumes that header, domain_bits, and range_bits have already
    // been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {
      return common::deserialize_key(domain_bits,
                                     range_bits,
                                     party,
                                     s,
                                     cw,
                                     cw_last,
                                     in);
    }
  };

  static constexpr key_type make_key() noexcept {
    return key_type();
  }

  static std::unique_ptr<key_type> new_key() {
    return std::unique_ptr<key_type>(new key_type);
  }

  static key_type & as_key(void * const key) noexcept {
    assert(key != nullptr);
    return *static_cast<key_type *>(key);
  }

  static void delete_key(void * const key) noexcept {
    std::unique_ptr<key_type>(&as_key(key));
  }

  // TODO:
  // For line 12, there's a question of whether using a lookup table
  // versus ternary expressions gives more performance.
  static void gen(key_type key[2],
                  domain_type const & domain_slot,
                  range_type range_value,
                  rand_perm_type & pi,
                  rng_type & rng) noexcept {
    common::gen(domain_bits,
                range_bits,
                n_minus_v,
                v,
                key,
                domain_slot,
                range_value,
                pi,
                rng);
  }

  static range_type eval(key_type const & key,
                         domain_type const & x,
                         rand_perm_type & pi) noexcept {
    return common::eval(domain_bits,
                        range_bits,
                        n_minus_v,
                        v,
                        key,
                        x,
                        pi);
  }

  using pack_eval_cache_t = std::array<typename common::cache_entry_t, v>;

  static pack_eval_cache_t make_pack_eval_cache() {
    return pack_eval_cache_t();
  }

  template<class OutputIt>
  static OutputIt pack_eval(key_type const & key,
                            domain_type const & x,
                            rand_perm_type & pi,
                            pack_eval_cache_t & cache,
                            int const b,
                            OutputIt const out) noexcept {
    return common::pack_eval(domain_bits,
                             range_bits,
                             n_minus_v,
                             v,
                             pack_count,
                             key,
                             cache,
                             b,
                             x,
                             pi,
                             out);
  }

#if 0
  template<class OutputIt>
  static OutputIt pack_eval_x4(key_type const & key,
                               domain_type const & x0,
                               domain_type const & x1,
                               domain_type const & x2,
                               domain_type const & x3,
                               rand_perm_type & pi,
                               OutputIt const out) noexcept {
    return common::pack_eval_x4(domain_bits,
                                range_bits,
                                n_minus_v,
                                v,
                                pack_count,
                                key,
                                x0,
                                x1,
                                x2,
                                x3,
                                pi,
                                out);
  }
#endif

  template<class RangeTypeContainer>
  static void eval_all(key_type const & key,
                       RangeTypeContainer & ys,
                       rand_perm_type & pi) {
    auto const domain_max =
        std::numeric_limits<domain_type>::max()
        >> (std::numeric_limits<domain_type>::digits - domain_bits);
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

//
// Runtime domain_bits and range_bits.
//

template<class DomainType,
         class RangeType,
         class BlockType,
         class RandPermType,
         class RngType,
         class RangePack = std::true_type,
         class PrgConvert = std::true_type>
class bgi1_v final {

  using common = bgi1_common<DomainType,
                             RangeType,
                             BlockType,
                             RandPermType,
                             RngType,
                             RangePack,
                             PrgConvert>;

public:
  int const domain_bits;
  int const range_bits;
  using domain_type = typename common::domain_type;
  using range_type = typename common::range_type;
  using block_type = typename common::block_type;
  using rand_perm_type = typename common::rand_perm_type;
  using rng_type = typename common::rng_type;
  static constexpr bool range_pack = common::range_pack;
  static constexpr bool prg_convert = common::prg_convert;

  static constexpr int range_type_bits = common::range_type_bits;
  using nice_range_type = typename common::nice_range_type;
  static constexpr int block_bits = common::block_bits;
  static constexpr int cw_last_bits = common::cw_last_bits;

  range_type const range_mask;
  int const n_minus_v;
  int const v;
  int const pack_count;
  int const pack_mask;

private:
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

  static block_type zero_msb(block_type x) {
    x = x.set_msb(0);
    return x;
  }

public:
  bgi1_v(int const domain_bits, int const range_bits) noexcept
      : domain_bits(domain_bits),
        range_bits(range_bits),
        range_mask(get_mask<range_type>(range_bits)),
        n_minus_v(min(
            domain_bits,
            range_pack ? power_of_two_width(cw_last_bits / range_bits) :
                         0)),
        v(domain_bits - n_minus_v),
        pack_count(1 << n_minus_v),
        pack_mask(pack_count - 1) {
    assert(domain_bits > 0);
    assert(range_bits > 0);
    assert(range_bits <= range_type_bits);
  }

  ~bgi1_v() noexcept = default;
  bgi1_v(bgi1_v const &) noexcept = default;
  bgi1_v(bgi1_v &&) noexcept = default;
  bgi1_v & operator=(bgi1_v const &) noexcept = default;
  bgi1_v & operator=(bgi1_v &&) noexcept = default;

  static std::array<unsigned char, 16> const & fixed_key() {
    return fixed_aes_128_key;
  }

  constexpr std::size_t key_blob_size() const noexcept {
    return key_type(domain_bits, range_bits, n_minus_v, v).size();
  }

  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  using cw_type = typename common::cw_type;

  struct key_type final {

    int const domain_bits;
    int const range_bits;
    int const n_minus_v;
    int const v;
    bool party;
    block_type s;
    std::vector<cw_type> cw;
    std::vector<range_type> cw_last;

    key_type(int const domain_bits,
             int const range_bits,
             int const n_minus_v,
             int const v)
        : domain_bits(domain_bits),
          range_bits(range_bits),
          n_minus_v(n_minus_v),
          v(v),
          cw(v),
          cw_last(1 << n_minus_v) {
    }

    constexpr std::size_t size() const noexcept {
      return common::key_blob_size(domain_bits,
                                   range_bits,
                                   n_minus_v,
                                   v);
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      return common::serialize_key(domain_bits,
                                   range_bits,
                                   party,
                                   s,
                                   cw,
                                   cw_last,
                                   out);
    }

    // Assumes that header, domain_bits, and range_bits have already
    // been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {
      return common::deserialize_key(domain_bits,
                                     range_bits,
                                     party,
                                     s,
                                     cw,
                                     cw_last,
                                     in);
    }
  };

  key_type make_key() const noexcept {
    return key_type(domain_bits, range_bits, n_minus_v, v);
  }

  std::unique_ptr<key_type> new_key() const {
    return std::unique_ptr<key_type>(
        new key_type(domain_bits, range_bits, n_minus_v, v));
  }

  static key_type & as_key(void * const key) noexcept {
    assert(key != nullptr);
    return *static_cast<key_type *>(key);
  }

  static void delete_key(void * const key) noexcept {
    std::unique_ptr<key_type>(&as_key(key));
  }

  // TODO:
  // For line 12, there's a question of whether using a lookup table
  // versus ternary expressions gives more performance.
  void gen(key_type key[2],
           domain_type const & domain_slot,
           range_type const range_value,
           rand_perm_type & pi,
           rng_type & rng) const noexcept {
    common::gen(domain_bits,
                range_bits,
                n_minus_v,
                v,
                key,
                domain_slot,
                range_value,
                pi,
                rng);
  }

  range_type eval(key_type const & key,
                  domain_type const & x,
                  rand_perm_type & pi) const noexcept {
    return common::eval(domain_bits,
                        range_bits,
                        n_minus_v,
                        v,
                        key,
                        x,
                        pi);
  }

  using pack_eval_cache_t = std::vector<typename common::cache_entry_t>;

  pack_eval_cache_t make_pack_eval_cache() const {
    return pack_eval_cache_t(v);
  }

  template<class OutputIt>
  OutputIt pack_eval(key_type const & key,
                     domain_type const & x,
                     rand_perm_type & pi,
                     pack_eval_cache_t & cache,
                     int const b,
                     OutputIt const out) const noexcept {
    return common::pack_eval(domain_bits,
                             range_bits,
                             n_minus_v,
                             v,
                             pack_count,
                             key,
                             cache,
                             b,
                             x,
                             pi,
                             out);
  }

#if 0
  template<class OutputIt>
  OutputIt pack_eval_x4(key_type const & key,
                        domain_type const & x0,
                        domain_type const & x1,
                        domain_type const & x2,
                        domain_type const & x3,
                        rand_perm_type & pi,
                        OutputIt const out) const noexcept {
    return common::pack_eval_x4(domain_bits,
                                range_bits,
                                n_minus_v,
                                v,
                                pack_count,
                                key,
                                x0,
                                x1,
                                x2,
                                x3,
                                pi,
                                out);
  }
#endif
};

} // namespace pfss

#endif // PFSS_BGI1_H
