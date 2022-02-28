//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BGI1_Z2N_H
#define PFSS_BGI1_Z2N_H

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
// Note: there are two key differences between this file and bgi1.h:
//   1. The group G used for the range is (Z_2)^N, where + and - operations
//      are defined as bitwise XOR. The group is implemented as the z2n class
//      (TODO).
//   2. Currently, the use case for this file calls for 2*lambda range bits.
//      Because of the large range type, this file does not use the "early termination"
//      optimization in gen.

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

template<int RangeBits>
class z2n final {
  static constexpr int range_bits = RangeBits;
  PFSS_SST_STATIC_ASSERT(range_bits > 0);

  std::array<unsigned char, bits_to_bytes(RangeBits)> data_;

public:
  //--------------------------------------------------------------------
  // Container access
  //--------------------------------------------------------------------

  using size_type = unsigned int;

  size_type size() const {
    return data_.size();
  }

  unsigned char & operator[](size_type const i) {
    assert(i < size());
    return data_[i];
  }

  unsigned char const & operator[](size_type const i) const {
    assert(i < size());
    return data_[i];
  }

  //--------------------------------------------------------------------
  // Arithmetic
  //--------------------------------------------------------------------
  z2n operator^(z2n const & other) const {
    assert(size() == other.size());
    z2n<range_bits> result;
    for (size_type i = 0; i < size(); i++) {
      result[i] = static_cast<unsigned char>((*this)[i] ^ other[i]);
    }
    return result;
  }

  z2n operator+(z2n const & other) const {
    return *this ^ other;
  }

  z2n operator-(z2n const & other) const {
    return *this ^ other;
  }

  z2n & operator=(z2n const & other) {
    assert(size() == other.size());
    for (size_type i = 0; i < other.size(); i++) {
      this->data_[i] = other.data_[i];
    }
    return *this;
  }

  bool operator==(z2n const & other) const {
    assert(size() == other.size());
    return std::equal(data_.begin(), data_.end(), other.data_.begin());
  }

  bool operator!=(z2n const & other) const {
    return !(*this == other);
  }
};

class z2n_v final {
  std::vector<unsigned char> data_;

public:
  int range_bits_;

  z2n_v(int const RangeBits) : range_bits_((assert(RangeBits > 0), RangeBits)) {
    data_.resize(
        sst::checked_cast<decltype(data_)::size_type>(
            bits_to_bytes(range_bits_)));
  }

  z2n_v() : range_bits_(1) {}

  //--------------------------------------------------------------------
  // Container access
  //--------------------------------------------------------------------

  using size_type = unsigned int;

  size_type size() const {
    return data_.size();
  }

  unsigned char & operator[](size_type const i) {
    assert(i < size());
    return data_[i];
  }

  unsigned char const & operator[](size_type const i) const {
    assert(i < size());
    return data_[i];
  }

  //
  // Returns the i'th bit of the value, where the LSB is the 0'th bit.
  // The value is treated as if it had infinitely many bits, i.e., the
  // function returns zero if i >= size * uchar_bits.
  //

  template<class T>
  bool getbitx(T const i) const noexcept {
    PFSS_SST_STATIC_ASSERT(std::is_integral<T>::value);
    assert(i >= 0);
    using U = typename promote_unsigned<T>::type;
    U const q = static_cast<U>(i / uchar_bits);
    U const r = static_cast<U>(i % uchar_bits);
    if (q < size()) {
      return (data_[q] >> r) & 1;
    }
    return 0;
  }

  //--------------------------------------------------------------------
  // Arithmetic
  //--------------------------------------------------------------------

  z2n_v operator^(z2n_v const & other) const {
    assert(range_bits_ == other.range_bits_);
    assert(size() == other.size());
    z2n_v result(range_bits_);
    for (size_type i = 0; i < size(); i++) {
      result[i] = static_cast<unsigned char>((*this)[i] ^ other[i]);
    }
    return result;
  }

  z2n_v operator+(z2n_v const & other) const {
    return *this ^ other;
  }

  z2n_v operator-(z2n_v const & other) const {
    return *this ^ other;
  }

  z2n_v & operator=(z2n_v const & other) {
    this->range_bits_ = other.range_bits_;
    this->data_.resize(0);
    for (size_type i = 0; i < other.size(); i++) {
      this->data_.push_back(other.data_[i]);
    }
    return *this;
  }

  bool operator==(z2n_v const & other) const {
    assert(range_bits_ == other.range_bits_);
    assert(size() == other.size());
    return std::equal(data_.begin(), data_.end(), other.data_.begin());
  }

  bool operator!=(z2n_v const & other) const {
    return !(*this == other);
  }

  z2n_v operator~() const {
    z2n_v result(range_bits_);
    for (std::size_t i = 0; i < this->data_.size(); i++) {
      result[i] = ~(this->data_[i]);
    }
    return result;
  }

  // Concatenate two z2n's together.
  z2n_v operator||(z2n_v const & other) const {
    z2n_v result(this->range_bits_ + other.range_bits_);
    unsigned char mask = 1;
    for (int i = 0; i < other.range_bits_; i++) {
      if ((i % uchar_bits) == 0) {
        mask = 1;
      } else {
        mask = static_cast<unsigned char>(mask << 1);
      }

      std::size_t byte_idx = static_cast<std::size_t>(i / uchar_bits);
      if (other.getbitx(i)) {
        result[byte_idx] = static_cast<unsigned char>(result[byte_idx] | mask);
      } else {
        result[byte_idx] = static_cast<unsigned char>(result[byte_idx] & (~mask));
      }
    }

    for (int i = 0; i < this->range_bits_; i++) {
      if (((i + other.range_bits_) % uchar_bits) == 0) {
        mask = 1;
      } else {
        mask = static_cast<unsigned char>(mask << 1);
      }

      std::size_t byte_idx = static_cast<std::size_t>((i + other.range_bits_) / uchar_bits);
      if (this->getbitx(i)) {
        result[byte_idx] = static_cast<unsigned char>(result[byte_idx] | mask);
      } else {
        result[byte_idx] = static_cast<unsigned char>(result[byte_idx] & (~mask));
      }
    }

    return result;
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class OutputIt>
  OutputIt serialize(OutputIt out) const {
    for (size_type i = 0; i != size(); ++i) {
      *out++ = (*this)[i];
    }
    return out;
  }

  template<class InputIt>
  InputIt parse(InputIt in) {
    for (size_type i = 0; i != size(); ++i) {
      (*this)[i] = *in++;
    }
    return in;
  }
};

template<class DomainType,
         class RangeType,
         class BlockType,
         class RandPermType,
         class RngType>
struct bgi1_z2n_common {
  using domain_type = typename remove_cvref<DomainType>::type;
  using range_type = typename remove_cvref<RangeType>::type;
  using block_type = typename remove_cvref<BlockType>::type;
  using rand_perm_type = typename remove_cvref<RandPermType>::type;
  using rng_type = typename remove_cvref<RngType>::type;

  static constexpr int block_bits = block_type::bits;

  // Assumes that the result variable is sized properly
  // to contain a range value.
  static void convert(int const & range_bits,
                      RandPermType & pi,
                      block_type s,
                      range_type & result) {
    // std::size_t range_bytes =  range_bits / uchar_bits +
    //     (((range_bits % uchar_bits) == 0) ? 0 : 1);
    if (result.size() <= s.size()) {
      for (std::size_t i = 0; i < result.size(); i++) {
        result[i] = static_cast<decltype(result[i])>(s[i]);
      }
    } else {
      std::vector<block_type> prg_result;
      std::size_t num_blocks =
          result.size() / s.size()
          + (((result.size() % s.size()) == 0) ? 0 : 1);
      for (std::size_t i = 0; i < num_blocks; i++) {
        block_type offset = block_type::from_uint(i);
        s ^= offset;
        prg_result.push_back(pi(s) ^ s);
      }

      std::size_t block_idx = 0;
      std::size_t byte_idx = 0;
      for (std::size_t i = 0; i < result.size(); i++) {
        result[i] = static_cast<decltype(result[i])>(
            prg_result[block_idx][byte_idx]);
        if (byte_idx == s.size()) {
          byte_idx = 0;
          block_idx++;
        }
      }
    }
    if (range_bits % uchar_bits != 0) {
      result[result.size() - 1] &= get_mask<unsigned char>(
          uchar_bits - (range_bits % uchar_bits));
    }
  }

  struct cw_type {
    block_type scw;
    std::array<bool, 2> tcw;
  };

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {5};
  }

  static constexpr std::size_t
  key_blob_size(int const domain_bits, int const range_bits) noexcept {
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
      + T(domain_bits) * T(block_type().size())

      // The party bit and the tcw bits.
      + T(1)
      + T(2) * T(bits_to_bytes(domain_bits))

      // The cw_last values.
      + T(bits_to_bytes(range_bits))

    );
    // clang-format on
  }

  template<class CwTypeContainer, class ContiguousByteOutputIt>
  static ContiguousByteOutputIt
  serialize_key(int const domain_bits,
                int const range_bits,
                bool const party,
                block_type const & s,
                CwTypeContainer const & cw,
                range_type const & cw_last,
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
    out = cw_last.serialize(out);
    return out;
  }

  // Assumes that header, domain_bits, and range_bits have already
  // been parsed and verified, and that the blob size has also been
  // verified.
  template<class CwTypeContainer, class ContiguousByteInputIt>
  static ContiguousByteInputIt
  deserialize_key(int const domain_bits,
                  int const range_bits,
                  bool & party,
                  block_type & s,
                  CwTypeContainer & cw,
                  range_type & cw_last,
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

    in = cw_last.parse(in);

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
    for (int i = 0; i != domain_bits; ++i) {
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
    range_type convert_s0(range_bits);
    range_type convert_s1(range_bits);
    convert(range_bits, pi, s[0], convert_s0);
    convert(range_bits, pi, s[1], convert_s1);
    key[0].cw_last = beta - convert_s0 + convert_s1;

    // key[0].cw_last[i] &= get_mask<range_type>(range_bits);
    key[1].cw_last = key[0].cw_last;

    // Line 16 doesn't happen because we already constructed the keys on
    // Lines 2, 11, and 15.
  }

  template<class KeyType>
  static range_type eval(int const domain_bits,
                         int const range_bits,
                         KeyType const & key,
                         domain_type const & x,
                         rand_perm_type & pi) noexcept {
    block_type s = key.s;
    bool t = key.party;
    for (int i = 0; i != domain_bits; ++i) {
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
    range_type y(range_bits);
    convert(range_bits, pi, s, y);

    if (t) {
      return y + key.cw_last;
    } else {
      return y;
    }
  }
};

template<int DomainBits,
         int RangeBits,
         class DomainType,
         class RangeType,
         class BlockType = default_block_t<>,
         class RandPermType = default_rand_perm_t<BlockType>,
         class RngType = default_rng_t<BlockType>>
class bgi1_z2n final {

  using common = bgi1_z2n_common<DomainType,
                                 RangeType,
                                 BlockType,
                                 RandPermType,
                                 RngType>;

public:
  static constexpr int domain_bits = DomainBits;
  static constexpr int range_bits = RangeBits;
  using domain_type = typename common::domain_type;
  using range_type = typename common::range_type;
  using block_type = typename common::block_type;
  using rand_perm_type = typename common::rand_perm_type;
  using rng_type = typename common::rng_type;

  static constexpr int block_bits = common::block_bits;

  PFSS_SST_STATIC_ASSERT(domain_bits > 0);
  PFSS_SST_STATIC_ASSERT(range_bits > 0);

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
  bgi1_z2n() noexcept = default;
  bgi1_z2n(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bgi1_z2n() noexcept = default;
  bgi1_z2n(bgi1_z2n const &) noexcept = default;
  bgi1_z2n(bgi1_z2n &&) noexcept = default;
  bgi1_z2n & operator=(bgi1_z2n const &) noexcept = default;
  bgi1_z2n & operator=(bgi1_z2n &&) noexcept = default;

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
    std::array<cw_type, DomainBits> cw;
    range_type cw_last;

    static constexpr std::size_t size() noexcept {
      return common::key_blob_size(DomainBits, RangeBits);
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
                key,
                domain_slot,
                range_value,
                pi,
                rng);
  }

  static range_type eval(key_type const & key,
                         domain_type const & x,
                         rand_perm_type & pi) noexcept {
    return common::eval(domain_bits, range_bits, key, x, pi);
  }

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
         class RngType>
class bgi1_z2n_v final {

  using common = bgi1_z2n_common<DomainType,
                                 RangeType,
                                 BlockType,
                                 RandPermType,
                                 RngType>;

public:
  int const domain_bits;
  int const range_bits;
  using domain_type = typename common::domain_type;
  using range_type = typename common::range_type;
  using block_type = typename common::block_type;
  using rand_perm_type = typename common::rand_perm_type;
  using rng_type = typename common::rng_type;

  static constexpr int block_bits = common::block_bits;

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
  bgi1_z2n_v(int const domain_bits, int const range_bits) noexcept
      : domain_bits(domain_bits),
        range_bits(range_bits) {
    assert(domain_bits > 0);
    assert(range_bits > 0);
  }

  ~bgi1_z2n_v() noexcept = default;
  bgi1_z2n_v(bgi1_z2n_v const &) noexcept = default;
  bgi1_z2n_v(bgi1_z2n_v &&) noexcept = default;
  bgi1_z2n_v & operator=(bgi1_z2n_v const &) noexcept = default;
  bgi1_z2n_v & operator=(bgi1_z2n_v &&) noexcept = default;

  static std::array<unsigned char, 16> const & fixed_key() {
    return fixed_aes_128_key;
  }

  constexpr std::size_t key_blob_size() const noexcept {
    return key_type(domain_bits, range_bits).size();
  }

  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  using cw_type = typename common::cw_type;

  struct key_type final {

    int domain_bits;
    int range_bits;
    bool party;
    block_type s;
    std::vector<cw_type> cw;
    range_type cw_last;

    key_type(int const domain_bits, int const range_bits)
        : domain_bits(domain_bits),
          range_bits(range_bits),
          cw_last(range_bits) {
      cw = std::vector<cw_type>(domain_bits);
    }

    constexpr std::size_t size() const noexcept {
      return common::key_blob_size(domain_bits, range_bits);
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

    key_type & operator=(key_type const & other) noexcept {
      this->domain_bits = other.domain_bits;
      this->range_bits = other.range_bits;
      this->party = other.party;
      this->s = other.s;
      this->cw = other.cw;
      this->cw_last = other.cw_last;
      return *this;
    }
  };

  key_type make_key() const noexcept {
    return key_type(domain_bits, range_bits);
  }

  std::unique_ptr<key_type> new_key() const {
    return std::unique_ptr<key_type>(
        new key_type(domain_bits, range_bits));
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
                key,
                domain_slot,
                range_value,
                pi,
                rng);
  }

  range_type eval(key_type const & key,
                  domain_type const & x,
                  rand_perm_type & pi) const noexcept {
    return common::eval(domain_bits, range_bits, key, x, pi);
  }

  template<class RangeTypeContainer>
  void eval_all(key_type const & key,
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

} // namespace pfss

#endif // PFSS_BGI1_Z2N_H
