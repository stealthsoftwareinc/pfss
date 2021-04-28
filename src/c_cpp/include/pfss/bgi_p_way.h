//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BGIP_H
#define PFSS_BGIP_H

//
// This file implements Algorithms 3 and 4 from
// <https://doi.org/10.1007/978-3-662-46803-6_12>. For reference, here
// is a complete citation:
//
//       Boyle E., Gilboa N., Ishai Y. (2015) Function Secret
//       Sharing. In: Oswald E., Fischlin M. (eds) Advances in
//       Cryptology - EUROCRYPT 2015. EUROCRYPT 2015. Lecture
//       Notes in Computer Science, vol 9057. Springer, Berlin,
//       Heidelberg.
//       https://doi.org/10.1007/978-3-662-46803-6_12
//

// Include first to test independence.
#include <pfss.h>
// Include twice to test idempotence.
#include <pfss.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <math.h>
#include <memory>
#include <random>
#include <stdexcept>
#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>

#include <pfss/common.h>
#include <pfss/mu_lut.hpp>

namespace pfss {

template<class DomainType,
         class RangeType,
         class RandBytesBufIt,
         class BlockType,
         class RandPermType,
         class RngType>
struct bgip_common {
  using domain_type = typename remove_cvref<DomainType>::type;
  using range_type = typename remove_cvref<RangeType>::type;
  using block_type = typename remove_cvref<BlockType>::type;
  using rand_perm_type = typename remove_cvref<RandPermType>::type;
  using rng_type = typename remove_cvref<RngType>::type;

  PFSS_STATIC_ASSERT(is_unsigned_integer<range_type>::value);

  static constexpr int range_type_bits = type_bits<range_type>();
  using nice_range_type = typename promote_unsigned<range_type>::type;
  static constexpr int block_bits = block_type::bits;

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {2};
  }

  static constexpr std::size_t
  key_blob_size(int const num_parties,
                std::size_t const nu,
                std::size_t const num_blocks) noexcept {
    // clang-format off
    return static_cast<std::size_t>(key_header().size())

      // The domain_bits value.
      + 1

      // The range_bits value.
      + 1

      // The party value.
      + 1

      // The prg seeds.
      + block_type().size() * nu * (static_cast<std::size_t>(1) << (num_parties - 1))

      // The correction words.
      + block_type().size() * (static_cast<std::size_t>(1) << (num_parties - 1)) * num_blocks
    ;
    // clang-format on
  }

  template<class ContiguousByteOutputIt>
  static ContiguousByteOutputIt
  serialize_key(int const domain_bits,
                int const range_bits,
                int const party,
                std::vector<std::vector<block_type>> const & sigma,
                std::vector<std::vector<block_type>> const & cws,
                ContiguousByteOutputIt out) noexcept {
    auto const h = key_header();
    out = std::copy(h.cbegin(), h.cend(), out);
    out = serialize_integer(uchar_bits, domain_bits, out);
    out = serialize_integer(uchar_bits, range_bits, out);
    out = serialize_integer(uchar_bits, party, out);
    for (auto & sigma_i : sigma) {
      for (auto & seed : sigma_i) {
        out = seed.serialize(out);
      }
    }

    for (auto & cw_row : cws) {
      for (auto & cw : cw_row) {
        out = cw.serialize(out);
      }
    }
    return out;
  }

  // Assumes that header, domain_bits, range_bits, and party id have
  // already been parsed and verified, and that the blob size has also
  // been verified.
  template<class ContiguousByteInputIt>
  static ContiguousByteInputIt
  deserialize_key(int const domain_bits,
                  int const range_bits,
                  std::size_t const num_parties,
                  std::size_t const nu,
                  std::size_t const num_blocks,
                  std::vector<std::vector<block_type>> & sigma,
                  std::vector<std::vector<block_type>> & cws,
                  ContiguousByteInputIt in) {

    // Skip past the header.
    std::advance(in, key_header().size());

    unsigned char const db = *in++;
    if (unsigned_ne(db, domain_bits)) {
      throw std::invalid_argument("key blob is corrupt");
    }

    unsigned char const rb = *in++;
    if (unsigned_ne(rb, range_bits)) {
      throw std::invalid_argument("key blob is corrupt");
    }

    // Skip past the party id.
    in++;

    std::size_t num_cws = static_cast<std::size_t>(1)
                          << (num_parties - 1);
    sigma.resize(nu);
    for (auto & sigma_i : sigma) {
      sigma_i.resize(num_cws);
      for (auto & s : sigma_i) {
        in = s.parse(in);
      }
    }

    cws.resize(num_cws);
    for (auto & cw : cws) {
      cw.resize(num_blocks);
      for (auto & cw_block : cw) {
        in = cw_block.parse(in);
      }
    }

    return in;
  }

  // PRG must output a vector of blocks whose total size in bytes is >=
  // the number of range bits * mu.
  static void prg(std::size_t const & min_prg_output_size,
                  std::size_t const & num_blocks,
                  RandPermType & pi,
                  BlockType x,
                  std::vector<block_type> & result) {
    // Correlation robustness (GKWY-2019-074): pi(x) ^ x.
    if (x.size() >= min_prg_output_size) {
      result.resize(1);
      result[0] = pi(x);
    } else if (2 * x.size() >= min_prg_output_size) {
      result.resize(2);
      result[0] = pi(x) ^ x;

      x.flip_msb();
      result[1] = pi(x) ^ x;
    } else {
      result.resize(num_blocks);
      for (std::size_t i = 0; i < num_blocks; i++) {
        block_type offset = block_type::from_uint(i);
        x ^= offset;
        result[i] = pi(x) ^ x;
      }
    }
  }

  static constexpr std::size_t
  compute_min_prg_output_size(int const range_bits,
                              int const mu) noexcept {
    return (range_bits * mu) / uchar_bits
           + ((range_bits * mu) % uchar_bits == 0 ? 0 : 1);
  }

  static constexpr std::size_t
  compute_num_blocks(std::size_t const min_prg_output_size) noexcept {
    return min_prg_output_size / block_type::bytes
           + ((min_prg_output_size) % block_type::bytes == 0 ? 0 : 1);
  }

  // The following function counts the number of 1's inside of
  // the uint64_t binary integer input. Returns 1 if odd and
  // 0 if even.
  //
  // This function was taken from:
  // https://graphics.stanford.edu/~seander/bithacks.html
  static bool compute_parity(uint64_t input) {
    input ^= input >> 1;
    input ^= input >> 2;
    input = (input & 0x1111111111111111UL) * 0x1111111111111111UL;
    return (input >> 60) & 1;
  }

  // The following function samples uniformly at random an
  // integer T from either the set of all binary integer T's
  // with an even number 1's or the set of all binary integer
  // T's with an odd number of 1's. To do this, it takes in
  // a random iterator over bytes and returns the advanced
  // iterator along with the resuling sampled integer T stored
  // inside 'result'.
  template<class T>
  static RandBytesBufIt sample_ep_or_op(const int num_parties,
                                        const bool even,
                                        RandBytesBufIt it,
                                        T & result) {
    result = static_cast<T>(*it++);
    auto const n = value_bits<
        typename std::remove_reference<decltype(*it)>::type>::value;
    for (auto i = num_parties; unsigned_gt(i, n); i -= n) {
      result <<= n;
      result |= static_cast<T>(*it++);
    }
    result &= get_mask<T>(num_parties);
    result ^= (even == compute_parity(result));
    return it;
  }

  template<class KeyType>
  static void gen(int const domain_bits,
                  int const range_bits,
                  int const num_parties,
                  std::vector<KeyType> & key,
                  domain_type const & alpha,
                  range_type beta,
                  rand_perm_type & pi,
                  rng_type & rng,
                  RandBytesBufIt rand_buf) noexcept {
    const domain_type mu =
        compute_mu<domain_type>(domain_bits, num_parties);
    const auto nu = compute_nu<decltype(mu)>(domain_bits, mu);

    // Obtain the number of total bytes that are needed as output
    // from the prg.
    const std::size_t min_prg_output_size =
        compute_min_prg_output_size(range_bits, mu);
    const std::size_t num_blocks =
        compute_num_blocks(min_prg_output_size);

    // Algorithm 3, line 3.
    auto const domain_max =
        std::numeric_limits<domain_type>::max()
        >> (std::numeric_limits<domain_type>::digits - domain_bits);
    domain_type const gamma = (mu >= domain_max) ? 0 : (alpha / mu);
    domain_type const delta = (mu >= domain_max) ? alpha : (alpha % mu);

    // Algorithm 3, line 4. 1st dimension corresponds to A's subscript, second dimension corresponds
    // to the column value described in Notation 2, and the uint64_t corresponds to the party index
    // (namely, the p LSB's of each uint64_t) that represents the row of the array described in
    // Notation 2.
    // TODO: pick the min sized uint to contain p bits instead of hardcoding uint64_t.
    std::vector<std::vector<uint64_t>> bit_arrays(nu);
    const std::size_t num_columns = static_cast<std::size_t>(1)
                                    << (num_parties - 1);
    for (domain_type gamma_prime = 0; gamma_prime < nu; gamma_prime++) {
      // Sample A_gamma_prime
      // TODO: the elements of this vector can be a uint just large enough to hold p bits.
      std::vector<uint64_t> a_gamma_prime(num_columns);
      for (std::size_t col_i = 0; col_i < num_columns; col_i++) {
        typename decltype(a_gamma_prime)::value_type x;
        rand_buf = sample_ep_or_op<uint64_t>(num_parties,
                                             gamma_prime != gamma,
                                             rand_buf,
                                             x);
        a_gamma_prime[col_i] = x;
      }
      bit_arrays[gamma_prime] = a_gamma_prime;
    }

    // Algorithm 3, line 5.
    std::vector<std::vector<block_type>> s(
        nu,
        std::vector<block_type>(num_columns));

    for (domain_type gamma_prime = 0; gamma_prime < nu; gamma_prime++) {
      for (std::size_t col_i = 0; col_i < num_columns; col_i++) {
        s[gamma_prime][col_i] = rng();
      }
    }

    // Algorithm 3, line 6.
    // Create e_delta * beta from Algorithm 3, Line 6. This container
    // could be oversized.
    std::size_t block_size = s[0][0].size();
    std::vector<block_type> e_delta_beta(num_blocks);

    // Find the byte index and offset at which b begins.
    std::size_t byte_index = delta * range_bits / uchar_bits;
    std::size_t block_index = byte_index / e_delta_beta[0].size();
    std::size_t byte_offset = byte_index % e_delta_beta[0].size();
    std::size_t bit_offset = delta * range_bits % uchar_bits;

    // Copy beta's bytes into e_delta_beta, starting at the
    // byte_offset'th bit of the byte_index'th byte.
    unsigned char mask = static_cast<unsigned char>(1) << bit_offset;
    for (int i = 0; i < range_bits; i++) {
      if (mask == 0) {
        byte_offset++;
        mask = 1;
      }

      if (byte_offset == block_size) {
        block_index++;
        byte_offset = 0;
      }

      if (beta & 1) {
        e_delta_beta[block_index][byte_offset] |= mask;
      } else {
        e_delta_beta[block_index][byte_offset] &= ~mask;
      }

      beta >>= 1;
      mask = static_cast<unsigned char>(mask << 1);
    }

    // Create the CW's described in Algorithm 3, line 6.
    std::vector<std::vector<block_type>> cws(
        static_cast<std::size_t>(1) << (num_parties - 1),
        std::vector<block_type>(num_blocks, block_type()));
    std::vector<block_type> running_xor(num_blocks, block_type());
    std::vector<block_type> prg_output;
    for (std::size_t j = 0; j < cws.size(); j++) {
      prg(min_prg_output_size, num_blocks, pi, s[gamma][j], prg_output);
      for (std::size_t block_idx = 0; block_idx < num_blocks;
           block_idx++) {
        if (j != cws.size() - 1) {
          cws[j][block_idx] = rng();
          running_xor[block_idx] ^=
              (cws[j][block_idx] ^ prg_output[block_idx]);
        } else {
          cws[j][block_idx] = running_xor[block_idx]
                              ^ prg_output[block_idx]
                              ^ e_delta_beta[block_idx];
        }
      }
    }

    // Lines 7 and 8.
    std::vector<std::vector<std::vector<block_type>>> sigma(
        num_parties,
        std::vector<std::vector<block_type>>(nu));
    for (int i = 0; i < num_parties; i++) {
      // TODO: do not hard code this to uint64_t. See the TODO item above about
      // determining a uint to hold just p bits.
      uint64_t party_mask = static_cast<uint64_t>(1) << i;
      for (std::size_t gamma_prime = 0; gamma_prime < nu;
           gamma_prime++) {
        sigma[i][gamma_prime].resize(num_columns);
        for (std::size_t col_i = 0; col_i < num_columns; col_i++) {
          if (bit_arrays[gamma_prime][col_i] & party_mask) {
            sigma[i][gamma_prime][col_i] = s[gamma_prime][col_i];
          } else {
            sigma[i][gamma_prime][col_i] = block_type();
          }
        }
      }
    }

    // Lines 9 + 10.
    for (int i = 0; i < num_parties; i++) {
      key[i].party = i;
      key[i].cws = cws;
      key[i].sigma = sigma[i];
    }
  }

  template<class KeyType>
  static range_type eval(int const domain_bits,
                         int const range_bits,
                         int const num_parties,
                         KeyType const & key,
                         domain_type const & x,
                         rand_perm_type & pi) {
    const domain_type mu =
        compute_mu<domain_type>(domain_bits, num_parties);
    const auto nu = compute_nu<decltype(mu)>(domain_bits, mu);

    // Obtain the number of total bytes that are needed as output
    // from the prg.
    const std::size_t min_prg_output_size =
        compute_min_prg_output_size(range_bits, mu);
    const std::size_t num_blocks =
        compute_num_blocks(min_prg_output_size);

    // Algorithm 4, line 3.
    auto const domain_max =
        std::numeric_limits<domain_type>::max()
        >> (std::numeric_limits<domain_type>::digits - domain_bits);
    domain_type const gamma = (mu >= domain_max) ? 0 : (x / mu);
    domain_type const delta = (mu >= domain_max) ? x : (x % mu);

    const std::size_t num_cws = static_cast<std::size_t>(1)
                                << (num_parties - 1);
    std::vector<block_type> eval_result_in_here(num_blocks);
    std::vector<block_type> prg_output;
    for (std::size_t j = 0; j < num_cws; j++) {
      bool prg_seed_is_zero = true;
      for (std::size_t k = 0; k < key.sigma[gamma][j].size(); k++) {
        if (key.sigma[gamma][j][k] != 0) {
          prg_seed_is_zero = false;
        }
      }
      if (!prg_seed_is_zero) {
        prg(min_prg_output_size,
            num_blocks,
            pi,
            key.sigma[gamma][j],
            prg_output);

        for (std::size_t k = 0; k < prg_output.size(); k++) {
          eval_result_in_here[k] ^= key.cws[j][k] ^ prg_output[k];
        }
      }
    }

    nice_range_type y = 0;
    // Find the byte index and offset at which b begins.
    std::size_t byte_index = delta * range_bits / uchar_bits;
    std::size_t block_index =
        byte_index / eval_result_in_here[0].size();
    std::size_t byte_offset =
        byte_index % eval_result_in_here[0].size();
    std::size_t bit_offset = delta * range_bits % uchar_bits;

    // Copy eval result into y, starting at the
    // byte_offset'th bit of the byte_index'th byte.
    unsigned char mask = static_cast<unsigned char>(1) << bit_offset;
    nice_range_type result_mask = 1;
    std::size_t n_bytes_copied = 0;
    for (int i = 0; i < range_bits; i++) {
      if (mask == 0) {
        byte_offset++;
        n_bytes_copied++;
        mask = 1;
      }

      if (byte_offset == block_type().size()) {
        block_index++;
        byte_offset = 0;
      }

      bool output_bit =
          eval_result_in_here[block_index][byte_offset] & mask;
      if (output_bit) {
        y |= static_cast<nice_range_type>(result_mask);
      }

      mask = static_cast<unsigned char>(mask << 1);
      result_mask = static_cast<nice_range_type>(result_mask << 1);
    }

    return static_cast<range_type>(y);
  }
};

template<std::size_t NumParties,
         int DomainBits,
         int RangeBits,
         class DomainType,
         class RangeType,
         class RandBytesBufIt,
         class BlockType = default_block_t<>,
         class RandPermType = default_rand_perm_t<BlockType>,
         class RngType = default_rng_t<BlockType>>
class bgip final {
  using common = bgip_common<DomainType,
                             RangeType,
                             RandBytesBufIt,
                             BlockType,
                             RandPermType,
                             RngType>;

public:
  static constexpr int domain_bits = DomainBits;
  static constexpr int range_bits = RangeBits;
  using domain_type = DomainType;
  using range_type = RangeType;
  using block_type = BlockType;
  using rand_perm_type = RandPermType;
  using rng_type = RngType;

  using nice_range_type = typename promote_unsigned<range_type>::type;

  static constexpr int domain_type_bits =
      std::numeric_limits<domain_type>::digits;
  static constexpr int range_type_bits =
      std::numeric_limits<range_type>::digits;

private:
  static_assert(NumParties <= 64,
                "NumParties > 64 currently not supported");
  static_assert(NumParties >= 1, "NumParties must be at least 1");
  static_assert(DomainBits >= 1, "DomainBits must be at least 1");
  static_assert(std::is_unsigned<domain_type>::value,
                "domain_type must be an unsigned integer type");
  static_assert(!std::is_const<domain_type>::value,
                "domain_type must not be const");
  static_assert(!std::is_volatile<domain_type>::value,
                "domain_type must not be volatile");
  static_assert(!std::is_same<domain_type, bool>::value,
                "domain_type must not be bool");
  static_assert(domain_type_bits >= DomainBits,
                "domain_type must have at least DomainBits bits");

  static_assert(RangeBits >= 8, "RangeBits must be at least 8");
  static_assert(std::is_unsigned<range_type>::value,
                "range_type must be an unsigned integer type");
  static_assert(!std::is_const<range_type>::value,
                "range_type must not be const");
  static_assert(!std::is_volatile<range_type>::value,
                "range_type must not be volatile");
  static_assert(!std::is_same<range_type, bool>::value,
                "range_type must not be bool");
  static_assert(range_type_bits >= RangeBits,
                "range_type must have at least RangeBits bits");
  static_assert(
      RangeBits < block_type::bits,
      "RangeBits must be strictly less than block_type::bits");

  static constexpr domain_type mu =
      compute_mu<domain_type>(domain_bits, NumParties);
  static constexpr auto nu = compute_nu<decltype(mu)>(domain_bits, mu);

  // Obtain the number of total bytes that are needed as output
  // from the prg.
  static constexpr std::size_t min_prg_output_size =
      common::compute_min_prg_output_size(range_bits, mu);
  static constexpr std::size_t num_blocks =
      common::compute_num_blocks(min_prg_output_size);

public:
  bgip() noexcept = default;
  bgip(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bgip() noexcept = default;
  bgip(bgip const &) noexcept = default;
  bgip(bgip &&) noexcept = default;
  bgip & operator=(bgip const &) noexcept = default;
  bgip & operator=(bgip &&) noexcept = default;

  static std::array<unsigned char, 16> const & fixed_key() {
    return fixed_aes_128_key;
  }

  static constexpr std::size_t key_blob_size() noexcept {
    return key_type::size();
  }

  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size()
           * (static_cast<std::size_t>(1) << (NumParties - 1))
           * (nu + num_blocks - 1);
  }

  template<class T = uint64_t>
  static constexpr std::size_t ep_op_rand_buf_size() noexcept {
    return sizeof(T) * (static_cast<std::size_t>(1) << (NumParties - 1))
           * nu;
  }

  struct key_type final {

    static constexpr int domain_bits = DomainBits;
    static constexpr int range_bits = RangeBits;
    int party;
    std::vector<std::vector<block_type>> sigma;
    std::vector<std::vector<block_type>> cws;

    static constexpr std::size_t size() noexcept {
      // clang-format off
      return common::key_blob_size(NumParties, nu, num_blocks);
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      return common::serialize_key(domain_bits,
                                   range_bits,
                                   party,
                                   sigma,
                                   cws,
                                   out);
    }

    // Assumes that party id, header, domain_bits, and range_bits have
    // already been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {
      return common::deserialize_key(domain_bits,
                                     range_bits,
                                     NumParties,
                                     nu,
                                     num_blocks,
                                     sigma,
                                     cws,
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

  static void gen(std::vector<key_type> & key,
                  domain_type domain_slot,
                  range_type range_value,
                  rand_perm_type & pi,
                  rng_type & rng,
                  RandBytesBufIt rand_buf) {
    common::gen(domain_bits,
                range_bits,
                NumParties,
                key,
                domain_slot,
                range_value,
                pi,
                rng,
                rand_buf);
  }

  static range_type eval(key_type const & key,
                         domain_type const & x,
                         rand_perm_type & pi) {
    return common::eval(domain_bits,
                        range_bits,
                        NumParties,
                        key,
                        x,
                        pi);
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
//
//
//
//
// BGI Algorithms 3 and 4 (https://cs.idc.ac.il/~elette/FunctionSecretSharing.pdf)
//
//
//
//
//

template<class DomainType,
         class RangeType,
         class RandBytesBufIt,
         class BlockType = default_block_t<>,
         class RandPermType = default_rand_perm_t<BlockType>,
         class RngType = default_rng_t<BlockType>>
class bgip_v final {
  using common = bgip_common<DomainType,
                             RangeType,
                             RandBytesBufIt,
                             BlockType,
                             RandPermType,
                             RngType>;

public:
  using domain_type = DomainType;
  using range_type = RangeType;
  using block_type = BlockType;
  using rand_perm_type = RandPermType;
  using rng_type = RngType;

  using nice_range_type = typename promote_unsigned<range_type>::type;

  static constexpr int domain_type_bits =
      std::numeric_limits<domain_type>::digits;
  static constexpr int range_type_bits =
      std::numeric_limits<range_type>::digits;

private:
  static_assert(std::is_unsigned<domain_type>::value,
                "domain_type must be an unsigned integer type");
  static_assert(!std::is_const<domain_type>::value,
                "domain_type must not be const");
  static_assert(!std::is_volatile<domain_type>::value,
                "domain_type must not be volatile");
  static_assert(!std::is_same<domain_type, bool>::value,
                "domain_type must not be bool");

  static_assert(std::is_unsigned<range_type>::value,
                "range_type must be an unsigned integer type");
  static_assert(!std::is_const<range_type>::value,
                "range_type must not be const");
  static_assert(!std::is_volatile<range_type>::value,
                "range_type must not be volatile");
  static_assert(!std::is_same<range_type, bool>::value,
                "range_type must not be bool");

public:
  int const domain_bits;
  int const range_bits;
  std::size_t const NumParties;
  domain_type mu;
  domain_type nu;
  std::size_t min_prg_output_bits;
  std::size_t min_prg_output_size;
  std::size_t num_blocks;

  bgip_v(int const domain_bits,
         int const range_bits,
         int const num_parties) noexcept
      : domain_bits(domain_bits),
        range_bits(range_bits),
        NumParties(num_parties) {
    mu = compute_mu<domain_type>(domain_bits, num_parties);
    nu = compute_nu<decltype(mu)>(domain_bits, mu);

    // Obtain the number of total bytes that are needed as output
    // from the prg.
    min_prg_output_size =
        common::compute_min_prg_output_size(range_bits, mu);
    num_blocks = common::compute_num_blocks(min_prg_output_size);

    assert(domain_bits > 0);
    assert(domain_bits <= domain_type_bits);
    assert(range_bits >= 8);
    assert(range_bits <= range_type_bits);
    assert(range_bits < block_type::bits);
    assert(NumParties <= 64);
    assert(NumParties < domain_bits);
  }

  ~bgip_v() noexcept = default;
  bgip_v(bgip_v const &) noexcept = default;
  bgip_v(bgip_v &&) noexcept = default;
  bgip_v & operator=(bgip_v const &) noexcept = default;
  bgip_v & operator=(bgip_v &&) noexcept = default;

  static std::array<unsigned char, 16> const & fixed_key() {
    return fixed_aes_128_key;
  }

  std::size_t key_blob_size() const noexcept {
    return key_type(domain_bits, range_bits, NumParties, nu, num_blocks)
        .size();
  }

  std::size_t rand_buf_size() noexcept {
    return block_type().size()
           * (static_cast<std::size_t>(1) << (NumParties - 1))
           * (nu + num_blocks - 1);
  }

  template<class T = uint64_t>
  std::size_t ep_op_rand_buf_size() noexcept {
    return sizeof(T) * (static_cast<std::size_t>(1) << (NumParties - 1))
           * nu;
  }

  struct key_type final {

    int const domain_bits;
    int const range_bits;
    int party;
    std::vector<std::vector<block_type>> sigma;
    std::vector<std::vector<block_type>> cws;

    // Unlike the above member variables, these
    // three member variables are not serialized.
    int NumParties;
    std::size_t nu;
    std::size_t num_blocks;

    key_type(int const domain_bits,
             int const range_bits,
             int const NumParties,
             std::size_t const nu,
             std::size_t const num_blocks)
        : domain_bits(domain_bits),
          range_bits(range_bits),
          NumParties(NumParties),
          nu(nu),
          num_blocks(num_blocks) {
    }

    constexpr std::size_t size() noexcept {
      // clang-format off
      return common::key_blob_size(NumParties, nu, num_blocks);
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      return common::serialize_key(domain_bits,
                                   range_bits,
                                   party,
                                   sigma,
                                   cws,
                                   out);
    }

    // Assumes that party id, header, domain_bits, and range_bits have
    // already been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {
      return common::deserialize_key(domain_bits,
                                     range_bits,
                                     NumParties,
                                     nu,
                                     num_blocks,
                                     sigma,
                                     cws,
                                     in);
    }
  };

  key_type make_key() noexcept {
    return key_type(domain_bits,
                    range_bits,
                    NumParties,
                    nu,
                    num_blocks);
  }

  std::unique_ptr<key_type> new_key() const {
    return std::unique_ptr<key_type>(new key_type(domain_bits,
                                                  range_bits,
                                                  NumParties,
                                                  nu,
                                                  num_blocks));
  }

  static key_type & as_key(void * const key) noexcept {
    assert(key != nullptr);
    return *static_cast<key_type *>(key);
  }

  static void delete_key(void * const key) noexcept {
    std::unique_ptr<key_type>(&as_key(key));
  }

  void gen(std::vector<key_type> & key,
           domain_type domain_slot,
           range_type range_value,
           rand_perm_type & pi,
           rng_type & rng,
           RandBytesBufIt rand_buf) {
    common::gen(domain_bits,
                range_bits,
                NumParties,
                key,
                domain_slot,
                range_value,
                pi,
                rng,
                rand_buf);
  }

  range_type eval(key_type const & key,
                  domain_type const & x,
                  rand_perm_type & pi) {
    return common::eval(domain_bits,
                        range_bits,
                        NumParties,
                        key,
                        x,
                        pi);
  }
};

} // namespace pfss

#endif // PFSS_BGIP_H
