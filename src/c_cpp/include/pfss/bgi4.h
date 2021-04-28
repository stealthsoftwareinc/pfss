//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BGI4_H
#define PFSS_BGI4_H

//
// This file implements Figure 4 from <https://ia.cr/2018/707>. For
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
// The code differs from Figure 4 in the following ways:
//
// 1. In line 7 of Gen, all convert functions should be the convert*
//    variant
//
// 2. Each recursive call to traverse increments i, not decrement. This
//    ensures that we parse CW_v when the recursive stack reaches a
//    leaf node at depth v. The way the figure has it, it parses CW_0
//    when the recursive stack reaches depth v.
//
// 3. In line 8 of Traverse, Convert* should only take in s as argument,
//    and the result is summed with t*CW just like Eval from BGI1.
//    Also, this final result should be multiplied by -1 depending on
//    the party just like Eval from BGI1.
//
// 4. In pfss, the alpha_i that gets registered in each correction word
//    starts from LSB (alpha_0) to MSB (alpha_n or alpha_v) of the
//    secret domain location alpha. Thus, instead of how BGI4
//    manipulates j, the code's traverse function maintains the
//    recursive traversal path in its argument x, this argument is
//    analogous to BGI1's argument x. Note that if the traversal
//    terminates early due to the range value "packing" optimization,
//    the n-v MSB's of x will be all zeros.
//
// 5. To ensure that the proper range value gets stored in the proper
//    location in ys, the index into ys to store each of the 2^(n-v)
//    range values at each leaf node (call each of these range value
//    idx jprime) is created by storing the bits of jprime inside the
//    n-v MSB's of x (call this jprime||x). jprime||x is finally used
//    to index ys to store the jprime'th range value of the leaf node.
//

// Include first to test independence.
#include <pfss.h>
// Include twice to test idempotence.
#include <pfss.h>

#include <algorithm>
#include <array>
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

template<
    int DomainBits,
    int RangeBits,
    class DomainType,
    class RangeType,
    class BlockType = default_block_t<>,
    class RandPermType = default_rand_perm_t<BlockType>,
    class RngType = default_rng_t<BlockType>>
class bgi4 final {
  static constexpr bool L = 0;
  static constexpr bool R = 1;

  template<bool X>
  static BlockType prg(RandPermType & pi, BlockType x) {
    if (X == R) {
      x = x.flip_msb();
    }
    // Correlation robustness (GKWY-2019-074).
    return pi(x) ^ x;
  }

  static BlockType zero_msb(BlockType x) {
    x = x.set_msb(0);
    return x;
  }

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

  static constexpr int n_minus_v =
      min(DomainBits, power_of_two_width((block_type::bits - 1) / RangeBits));
  static_assert(n_minus_v >= 0, "power_of_two_width failed");
  static constexpr int v = DomainBits - n_minus_v;

  using convert_output = std::array<range_type, n_minus_v>;

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
  bgi4() noexcept = default;
  bgi4(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bgi4() noexcept = default;
  bgi4(bgi4 const &) noexcept = default;
  bgi4(bgi4 &&) noexcept = default;
  bgi4 & operator=(bgi4 const &) noexcept = default;
  bgi4 & operator=(bgi4 &&) noexcept = default;

  static std::array<unsigned char, 16> const & fixed_key() {
    return fixed_aes_128_key;
  }

  static constexpr std::size_t key_blob_size() noexcept {
    return key_type::size();
  }
  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  struct cwthing {
    block_type scw;
    bool tcw[2];
  };

  // TODO(Quinn): v=0 overflow problems? Use size_t?
  static constexpr auto cw_last_size = static_cast<domain_type>(1)
      << (DomainBits - v);

  struct key_type final {

    static constexpr std::array<unsigned char, 1> header() noexcept {
      return {1};
    }

    int domain_bits;
    int range_bits;
    bool party;
    block_type s;
    cwthing cw[v];
    std::array<range_type, cw_last_size> cw_last;

    static constexpr std::size_t size() noexcept {
      // clang-format off
      return static_cast<std::size_t>(header().size())

        // The domain_bits value.
        + 1

        // The range_bits value.
        + 1

        // The s block.
        + block_type().size()

        // The scw blocks.
        + v * static_cast<std::size_t>(block_type().size())

        // The party bit and tcw bits.
        + 1
        + 2 * static_cast<std::size_t>(v / uchar_bits + (v % uchar_bits == 0 ? 0 : 1))

        // The cw_last value.
        + bits_to_bytes(RangeBits) * static_cast<std::size_t>(cw_last_size)

      ;
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      auto const h = header();
      out = std::copy(h.cbegin(), h.cend(), out);
      out = serialize_integer(uchar_bits, domain_bits, out);
      out = serialize_integer(uchar_bits, range_bits, out);
      out = s.serialize(out);
      for (auto const & x : cw) {
        out = x.scw.serialize(out);
      }
      {
        *out++ = party;
        for (int j = 0; j != 2; ++j) {
          unsigned int d = 0;
          int i = 0;
          for (auto const & x : cw) {
            d |= static_cast<unsigned int>(x.tcw[j]) << i;
            if (++i == uchar_bits) {
              *out++ = static_cast<unsigned char>(d);
              d = 0;
              i = 0;
            }
          }
          if (i != 0) {
            *out++ = static_cast<unsigned char>(d);
          }
        }
      }
      for (auto const & x : cw_last) {
        out = serialize_integer(RangeBits, x, out);
      }
      return out;
    }

    // Assumes that header, domain_bits, and range_bits have already
    // been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {

      // Skip past the header.
      auto const h = header();
      for (decltype(h.size()) i = 0; i != h.size(); ++i) {
        ++in;
      }

      // Skip past domain_bits.
      ++in;
      domain_bits = DomainBits;

      // Skip past range_bits.
      ++in;
      range_bits = RangeBits;

      in = s.parse(in);

      for (int i = 0; i != v; ++i) {
        in = cw[i].scw.parse(in);
      }

      {
        party = *in++ & 1;
        for (int j = 0; j != 2; ++j) {
          int i = 0;
          for (auto & x : cw) {
            x.tcw[j] = (*in >> i) & 1;
            if (++i == uchar_bits) {
              ++in;
              i = 0;
            }
          }
          if (i != 0) {
            ++in;
          }
        }
      }

      for (range_type & x : cw_last) {
        in = deserialize_integer(RangeBits, x, in);
      }

      return in;
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
  static void
  gen(key_type key[2],
      domain_type domain_slot,
      range_type range_value,
      rand_perm_type & pi,
      rng_type & rng) {
    constexpr int n = DomainBits;
    domain_type const alpha = domain_slot;
    nice_range_type const beta = range_value;
    key[0].domain_bits = DomainBits;
    key[1].domain_bits = DomainBits;
    key[0].range_bits = RangeBits;
    key[1].range_bits = RangeBits;
    key[0].party = 0;
    key[1].party = 1;
    // Line 3.
    block_type s[2];
    s[0] = zero_msb(rng());
    s[1] = zero_msb(rng());
    // Line 8 (partial).
    key[0].s = s[0];
    key[1].s = s[1];
    // Line 4.
    bool t[2] = {0, 1};
    for (decltype(+v) i = 0; i != v; ++i) {
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
      bool const alpha_i = (alpha >> i) & 1;
      bool const keep = alpha_i;
      bool const lose = !alpha_i;
      block_type const scw = s0[lose] ^ s1[lose];
      bool tcw[2];
      tcw[L] = t0[L] ^ t1[L] ^ alpha_i ^ 1;
      tcw[R] = t0[R] ^ t1[R] ^ alpha_i;
      // Line 8 (partial).
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
    std::fill(key[0].cw_last.begin(), key[0].cw_last.end(), 0);
    key[0].cw_last
        [(v > 0 ? alpha >> (v - 1) >> 1 : alpha) &
         get_mask<domain_type>(n - v)] =  static_cast<range_type>(beta);
    auto const nnn = key[0].cw_last.size();
    for (decltype(+nnn) i = 0; i != nnn; ++i) {
      key[0].cw_last[i] = static_cast<range_type>(
          key[0].cw_last[i] - convert(s[0]) + convert(s[1]));
      if (t[1]) {
        key[0].cw_last[i] = static_cast<range_type>(-key[0].cw_last[i]);
      }
      key[0].cw_last[i] = static_cast<range_type>(key[0].cw_last[i] & get_mask<range_type>(RangeBits));
    }
    std::copy(
        key[0].cw_last.begin(),
        key[0].cw_last.end(),
        key[1].cw_last.begin());
  }

  template<class RangeTypeContainer>
  static void eval_all(
      key_type const & key,
      RangeTypeContainer & ys,
      rand_perm_type & pi) {
    // Line 3.
    traverse<RangeTypeContainer>(
        ys,
        key.s,
        key.party,
        key.cw,
        0,
        0,
        key.cw_last.data(),
        pi,
        key.party);
  }

  template<class RangeTypeContainer>
  static void traverse(
      RangeTypeContainer & ys, // container for range vals
      block_type const s, // the prg seed of the current node
      bool const t, // the control bit of the current node
      cwthing const * cw,
      int const i, // the ith bit of the traversal path
      domain_type const x, // the traversal path
      range_type const * cw_last, // the masked range vals
      rand_perm_type & pi,
      bool const & b) { // the party
    // Line 1.
    if (i < v) {
      // Left half of G's output from Line 4.
      block_type tau_l = prg<L>(pi, s);

      // Right half of G's output from Line 4.
      block_type tau_r = prg<R>(pi, s);

      // Line 4.
      if (t) {
        // scwl is the (lambda + 1)-bit portion: s_CW and tL_CW
        // from Line 4.
        block_type scwl = cw[i].scw;
        scwl = scwl.set_msb(cw[i].tcw[0]);

        // scwr is the (lambda + 1)-bit portion: s_CW and tR_CW
        // from Line 4.
        block_type scwr = cw[i].scw;
        scwr = scwr.set_msb(cw[i].tcw[1]);
        tau_l ^= scwl;
        tau_r ^= scwr;
      }
      bool tL = tau_l.get_msb();
      bool tR = tau_r.get_msb();
      tau_l = tau_l.set_msb(0);
      tau_r = tau_r.set_msb(0);
      // Line 6.
      traverse(ys, tau_l, tL, cw, i + 1, x, cw_last, pi, b);
      traverse(
          ys,
          tau_r,
          tR,
          cw,
          i + 1,
          x | (static_cast<domain_type>(1) << i),
          cw_last,
          pi,
          b);
    } else {
      auto const G_mask = get_mask<range_type>(range_bits);
      // Line 8.
      auto const convert_s = convert(s);
      for (decltype(+cw_last_size) jprime = 0; jprime < cw_last_size;
           jprime++) {
        domain_type ys_idx = x |
            (jprime > 0 ? static_cast<domain_type>(jprime) << v : 0);
        ys[ys_idx] = static_cast<typename RangeTypeContainer::value_type>(convert_s);
        if (t) {
          ys[ys_idx] = static_cast<typename RangeTypeContainer::value_type>(
              ys[ys_idx] + static_cast<nice_range_type>(cw_last[jprime]));
        }

        if (b == 1) { // if the party == 1
          ys[ys_idx] = static_cast<typename RangeTypeContainer::value_type>(-ys[ys_idx]);
        }
        ys[ys_idx] = static_cast<typename RangeTypeContainer::value_type>(ys[ys_idx] & G_mask);
      }
    }
  }
};

} // namespace pfss

#endif // PFSS_BGI4_H
