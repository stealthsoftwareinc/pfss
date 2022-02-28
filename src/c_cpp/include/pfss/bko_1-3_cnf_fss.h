//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_1_3_CNF_FSS_H
#define PFSS_1_3_CNF_FSS_H

//
// This file implements the 1 out of 3 secure CNF-FSS protocol from
// <https://eprint.iacr.org/2021/163>. For reference, here is a complete
// citation:
//
//       @misc{,
//           author = {Paul Bunn and Eyal Kushilevitz and Rafail Ostrovsky},
//           title = {CNF-FSS and its Applications},
//           howpublished = {Cryptology ePrint Archive: Report 2021/163},
//           year = {2021},
//           note = {\url{https://eprint.iacr.org/2021/163}},
//       }
//

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
// We use a security parameter of 126 because we use a PRG that
// stretches lambda bits into 2 * (lambda + 2) bits == 256 == the
// size of two 16-byte blocks, which is convenient given that i.e.
// an m128i block is a 16-byte block.
constexpr std::size_t bko_1_3_security_parameter = 126;

// Returns the max possible number of control bits needed for gen.
inline std::size_t num_control_bits(int const & range_bits) {
  return static_cast<std::size_t>(4 * range_bits);
}

// Returns the number of random bytes needed for the MS-DPF+
// protocols in gen.
inline std::size_t
num_rand_bytes_for_ms_dpf_plus(std::size_t const & lambda,
                               int const & domain_bits,
                               int const & range_bits) {
  return static_cast<std::size_t>(
      domain_bits * range_bits * 4
      * (bits_to_bytes(lambda * 2) + bits_to_bytes(lambda)));
}

namespace {
constexpr std::size_t lambda = bko_1_3_security_parameter;

// These arrays are temporary variables used to initialize the
// variables that contain the output of G.
template<class MsDpfPlusType>
struct ms_dpf_plus_range_type_array_lambda {
  std::array<typename MsDpfPlusType::range_type, 3> array_lambda = {
      typename MsDpfPlusType::range_type(lambda),
      typename MsDpfPlusType::range_type(lambda),
      typename MsDpfPlusType::range_type(lambda)};

  std::array<typename MsDpfPlusType::range_type, 2> array_lambda_two_d =
      {typename MsDpfPlusType::range_type(lambda),
       typename MsDpfPlusType::range_type(lambda)};
};

template<class MsDpfPlusType>
struct ms_dpf_plus_range_type_array_one {
  std::array<typename MsDpfPlusType::range_type, 3> array_one = {
      typename MsDpfPlusType::range_type(1),
      typename MsDpfPlusType::range_type(1),
      typename MsDpfPlusType::range_type(1)};
  std::array<typename MsDpfPlusType::range_type, 2> array_one_two_d = {
      typename MsDpfPlusType::range_type(1),
      typename MsDpfPlusType::range_type(1)};
};

// These are temporary vectors used to initialize the secret
// range value shares for MS-DPF+.

template<class MsDpfPlusType>
struct ms_dpf_plus_range_type_vector_lambda {
  std::vector<typename MsDpfPlusType::range_type> vector_lambda = {
      typename MsDpfPlusType::range_type(lambda),
      typename MsDpfPlusType::range_type(lambda),
      typename MsDpfPlusType::range_type(lambda)};
};

template<class MsDpfPlusType>
struct ms_dpf_plus_range_type_vector_two_lambda {
  std::vector<typename MsDpfPlusType::range_type> vector_two_lambda = {
      typename MsDpfPlusType::range_type(lambda * 2),
      typename MsDpfPlusType::range_type(lambda * 2),
      typename MsDpfPlusType::range_type(lambda * 2)};
};

// Sets the n MSBS's of b to zero.
template<class block_type>
void zero_msbs(std::size_t const & n, block_type & b) {
  assert(n <= b.bits);
  if (n == 0) {
    return;
  }

  std::size_t num_bytes = n / uchar_bits + (n % uchar_bits ? 1 : 0);
  unsigned char mask = get_mask<unsigned char>(uchar_bits - n);

  for (std::size_t byte_idx = b.size() - 1;
       byte_idx >= (b.size() - num_bytes);
       byte_idx--) {
    if (byte_idx == (b.size() - num_bytes)) {
      b[byte_idx] = static_cast<unsigned char>(b[byte_idx] & mask);
    } else {
      b[byte_idx] = 0;
    }
  }
}

template<class block_type, class MsDpfPlusRangeType>
void copy_block_to_ms_dpf_plus_range_type(
    block_type const & block,
    MsDpfPlusRangeType & destination) {
  for (std::size_t i = 0; i < block.size(); i++) {
    destination[i] = block[i];
  }
}

template<class block_type, class MsDpfPlusRangeType>
void copy_ms_dpf_plus_range_type_to_block(
    MsDpfPlusRangeType const & range_value,
    block_type & destination) {
  for (std::size_t i = 0; i < destination.size(); i++) {
    destination[i] = range_value[i];
  }
}

template<class block_type, class rng_type, class MsDpfPlusRangeType>
void construct_random_ms_dpf_plus_range_type(
    rng_type & rng,
    MsDpfPlusRangeType & result) {
  block_type p = rng();
  zero_msbs(p.bits - result.range_bits_, p);
  copy_block_to_ms_dpf_plus_range_type(p, result);
}

// Unlike the copy_block_to_ms_dpf_plus_range_type function, this function
// returns the range type containing the CNF-FSS range group element by
// copying the range_bits LSBs of the block to the range type.
template<class block_type, class nice_range_type>
nice_range_type block_to_range_type(int const & range_bits,
                                    block_type const & src) {
  PFSS_SST_STATIC_ASSERT(is_non_cvb_integer<nice_range_type>::value);
  assert(range_bits >= 1);
  assert(range_bits <= type_bits<nice_range_type>());
  nice_range_type dst = 0;
  int const nq = range_bits / uchar_bits;
  int const nr = range_bits % uchar_bits;
  int const n = nq + (nr != 0);
  for (int i = 0; i != n; ++i) {
    unsigned char b = src[i];
    if (nr != 0 && i == n - 1) {
      if (b > get_mask<unsigned char>(nr)) {
        b = static_cast<unsigned char>(b & get_mask<unsigned char>(nr));
      }
    }
    dst = static_cast<nice_range_type>(
        dst | (static_cast<nice_range_type>(b) << (uchar_bits * i)));
  }
  return dst;
}

// This function is used to take an MS-DPF+ range value of lambda * 2
// bits and splits the value into two MS-DPF+ range vaules; one
// consisting of to_split's lambda MSB's and another consisting of
// to_split's lambda LSB's.
template<class MsDpfPlusRangeType>
void split_ms_dpf_plus_range_type(MsDpfPlusRangeType const & to_split,
                                  MsDpfPlusRangeType & msbs,
                                  MsDpfPlusRangeType & lsbs) {
  unsigned char mask = 1;
  for (int i = 0; i < lsbs.range_bits_; i++) {
    if ((i % uchar_bits) == 0) {
      mask = 1;
    } else {
      mask = static_cast<unsigned char>(mask << 1);
    }

    std::size_t byte_idx = static_cast<std::size_t>(i / uchar_bits);
    if (to_split.getbitx(i)) {
      lsbs[byte_idx] =
          static_cast<unsigned char>(lsbs[byte_idx] | mask);
    } else {
      lsbs[byte_idx] =
          static_cast<unsigned char>(lsbs[byte_idx] & (~mask));
    }
  }
  for (int i = 0; i < msbs.range_bits_; i++) {
    if ((i % uchar_bits) == 0) {
      mask = 1;
    } else {
      mask = static_cast<unsigned char>(mask << 1);
    }

    std::size_t byte_idx = static_cast<std::size_t>(i / uchar_bits);
    if (to_split.getbitx(i + lsbs.range_bits_)) {
      msbs[byte_idx] =
          static_cast<unsigned char>(msbs[byte_idx] | mask);
    } else {
      msbs[byte_idx] =
          static_cast<unsigned char>(msbs[byte_idx] & (~mask));
    }
  }
}

// The ``ms_dpf_plus_rand_buf_it`` input parameter should point to the
// first unused element (within a buffer of random elements). This
// function returns the iterator (which has been advanced by however
// many random elements were consumed) so that it can be subsequently
// used again later (without re-using any of the already consumed
// elements).
template<class MsDpfPlusType,
         class domain_type,
         class rand_perm_type,
         class rng_type,
         class RandBytesBufIt,
         class key_type>
RandBytesBufIt generate_ms_dpf_keys(
    const bool is_hat,
    std::size_t const & k,
    std::size_t const & domain_bits,
    std::size_t const & range_bits,
    domain_type const & alpha,
    std::vector<typename MsDpfPlusType::range_type> const & v,
    rand_perm_type & pi,
    rng_type & rng,
    RandBytesBufIt ms_dpf_plus_rand_buf_it,
    key_type & key) {
  // Setup MS-DPF+ key containers for f.
  std::vector<typename MsDpfPlusType::key_type> kappa(
      3,
      typename MsDpfPlusType::key_type(domain_bits, range_bits));

  // Acquire randomness needed for MS-DPF+
  std::vector<typename MsDpfPlusType::range_type> rand_vprimes =
      is_hat ? ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                   .vector_lambda :
               ms_dpf_plus_range_type_vector_two_lambda<MsDpfPlusType>()
                   .vector_two_lambda;
  for (auto & vprime : rand_vprimes) {
    for (std::size_t i = 0; i < vprime.size(); i++) {
      vprime[i] =
          static_cast<unsigned char>(*ms_dpf_plus_rand_buf_it++);
    }
    if ((range_bits % uchar_bits) != 0) {
      vprime[vprime.size() - 1] &= get_mask<unsigned char>(
          uchar_bits - (range_bits % uchar_bits));
    }
  }

  // TODO: is it okay/safe that we use the same rng and pi for MS-DPF+?
  if (domain_bits > 0) {
    MsDpfPlusType MsDpfPlus(domain_bits, range_bits);
    MsDpfPlus.gen(kappa, alpha, v, rand_vprimes, pi, rng);
  }

  if (!is_hat) {
    for (std::size_t party = 0; party < 3; party++) {
      // Store the MS-DPF+ keys inside the CNF-FSS key. If we are only
      // on the first level of the binary tree, store the MS-DPF+ range
      // values in the clear.
      if (domain_bits > 0) {
        key[party][k].kappa.push_back(kappa[party]);
      }
    }
  } else {
    for (std::size_t party = 0; party < 3; party++) {
      key[party][k].kappahat.push_back(kappa[party]);
    }
  }
  return ms_dpf_plus_rand_buf_it;
}

// This function serves as the PRG used to stretch a lambda-bit
// seed into a 2 * (lambda + 2) bit output. The function,
// decompose_prg_output, defines which bits of the result
// map to which variables from BKO21.
template<class block_type, class rand_perm_type, int prg_output_size>
void prg(rand_perm_type & pi,
         block_type x,
         block_type result[prg_output_size]) {
  result[0] = pi(x) ^ x;
  x.flip_msb();
  result[1] = pi(x) ^ x;
}

// BKO21 defines the PRG: G(x_mu) = (G_L, H_L, Hhat_L), (G_R, H_R, Hhat_R),
// where G(x_mu) : {0, 1}^lambda -> {0, 1}^(2* (lambda + 2)); and
// G_L and G_R : {0, 1}^lambda -> {0, 1}^lambda; and
// H_L, Hhat_L, H_R, Hhat_R : {0, 1}^lambda -> {0, 1}.
//
// The function breaks G down by taking the 0th element of prg_output
// (which consists of lambda + 2 bits) and storing those bits into
// G_L, H_L and Hhat_L. H_L gets the MSB of prg_output[0], Hhat_L gets
// the second MSB of prg_output[0], and G_L gets the lambda LSB's of
// prg_output[0]. The analagous thing is done for the prg_output[1]
// and the rhs variables.
template<class block_type,
         class MsDpfPlusRangeType,
         int prg_output_size>
void decompose_prg_output(const block_type prg_output[prg_output_size],
                          MsDpfPlusRangeType & decomp_G_L,
                          MsDpfPlusRangeType & decomp_G_R,
                          MsDpfPlusRangeType & decomp_H_L,
                          MsDpfPlusRangeType & decomp_Hhat_L,
                          MsDpfPlusRangeType & decomp_H_R,
                          MsDpfPlusRangeType & decomp_Hhat_R) {
  copy_block_to_ms_dpf_plus_range_type(prg_output[0], decomp_G_L);
  decomp_H_L[0] = static_cast<unsigned char>(
      decomp_G_L[decomp_G_L.size() - 1] >> (uchar_bits - 1));
  assert(decomp_H_L[0] < 2);
  decomp_Hhat_L[0] = static_cast<unsigned char>(
      decomp_G_L[decomp_G_L.size() - 1] >> (uchar_bits - 2));
  decomp_Hhat_L[0] = static_cast<unsigned char>(decomp_Hhat_L[0] & 1);
  assert(decomp_Hhat_L[0] < 2);
  decomp_G_L[decomp_G_L.size() - 1] =
      decomp_G_L[decomp_G_L.size() - 1]
      & get_mask<unsigned char>(uchar_bits - 2);

  copy_block_to_ms_dpf_plus_range_type(prg_output[1], decomp_G_R);
  decomp_H_R[0] = static_cast<unsigned char>(
      decomp_G_R[decomp_G_R.size() - 1] >> (uchar_bits - 1));
  assert(decomp_H_R[0] < 2);
  decomp_Hhat_R[0] = static_cast<unsigned char>(
      decomp_G_R[decomp_G_R.size() - 1] >> (uchar_bits - 2));
  decomp_Hhat_R[0] = static_cast<unsigned char>(decomp_Hhat_R[0] & 1);
  assert(decomp_Hhat_R[0] < 2);
  decomp_G_R[decomp_G_R.size() - 1] =
      decomp_G_R[decomp_G_R.size() - 1]
      & get_mask<unsigned char>(uchar_bits - 2);
}

// WARNING: this convert function is only valid for groups of orders power of two.
// TODO: can we generalize this to Z_N for arbitrary N?
//
// The following convert function is parameterized by two lambda-bit seeds, x and s.
// Using these two seeds and a random permutation, pi, the function maps values
// x and s to an element of the range group.
template<class block_type, class rand_perm_type, class nice_range_type>
nice_range_type
prg_convert_for_power_of_two_groups(int const & range_bits,
                                    int const & num_group_bits,
                                    rand_perm_type const & pi,
                                    block_type const & x,
                                    block_type const & s) {
  block_type temp = x ^ s;
  return block_to_range_type<block_type, nice_range_type>(range_bits,
                                                          pi(temp)
                                                              ^ temp)
         % (static_cast<nice_range_type>(1) << num_group_bits);
}

// This function serves as a helper function to find the modular multiplicative
// inverse of a mod b. The result is stored in the variable pointed to by x, and
// y points to a temporary variable.
template<class nice_range_type>
nice_range_type gcdExtended(nice_range_type a,
                            nice_range_type b,
                            nice_range_type * x,
                            nice_range_type * y) {
  // Base Case
  if (a == 0) {
    *x = 0, *y = 1;
    return b;
  }

  // To store results of recursive call
  nice_range_type x1, y1;
  nice_range_type gcd = gcdExtended(b % a, a, &x1, &y1);

  // Update x and y using results of recursive
  // call
  *x = y1 - (b / a) * x1;
  *y = x1;

  return gcd;
}

// This function simultaneously finds a seed s and final correction
// word, w given the index k into the range group element bits,
// a random permutation pi, and the X and Y seeds of all three parties.
// s and w are used to calculate the resulting range element of eval.
template<class rand_perm_type,
         class rng_type,
         class block_type,
         class nice_range_type>
void find_cw(int const & range_bits,
             int const & k,
             std::size_t const & lambda,
             rand_perm_type & pi,
             rng_type & rng,
             block_type prg_input_xs[3],
             block_type prg_input_ys[3],
             block_type & s,
             nice_range_type & w) {
  nice_range_type gx1;
  nice_range_type gx2;
  nice_range_type gx3;
  nice_range_type gy1;
  nice_range_type gy2;
  nice_range_type gy3;
  nice_range_type m =
      (static_cast<nice_range_type>(1) << (range_bits - k));
  while (true) {
    s = rng();
    zero_msbs(s.bits - lambda, s);
    gx1 = prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
        range_bits,
        range_bits - k,
        pi,
        prg_input_xs[0],
        s);
    gx2 = prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
        range_bits,
        range_bits - k,
        pi,
        prg_input_xs[1],
        s);
    gx3 = prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
        range_bits,
        range_bits - k,
        pi,
        prg_input_xs[2],
        s);
    gy1 = prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
        range_bits,
        range_bits - k,
        pi,
        prg_input_ys[0],
        s);
    gy2 = prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
        range_bits,
        range_bits - k,
        pi,
        prg_input_ys[1],
        s);
    gy3 = prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
        range_bits,
        range_bits - k,
        pi,
        prg_input_ys[2],
        s);

    nice_range_type v = (gx1 + gx2 + gx3 - gy1 - gy2 - gy3) % m;
    if (v % 2 != 0) {
      nice_range_type temp;
      // Find the modular inverse of v mod m, and store it into w.
      gcdExtended(v, m, &w, &temp);
      w = (w % m + m) % m;
      return;
    }
  }
}

std::size_t party_to_the_right(std::size_t party_index) {
  assert(party_index <= 2);
  if (party_index == 2) {
    return 0;
  } else {
    return party_index + 1;
  }
}

std::size_t party_to_the_left(std::size_t party_index) {
  assert(party_index <= 2);
  if (party_index == 0) {
    return 2;
  } else {
    return party_index - 1;
  }
}
} // namespace

// CNF FSS class for domain and range bits specified at compile time.
template<int DomainBits,
         int RangeBits,
         class DomainType,
         class RangeType,
         class MsDpfPlusType,
         class RandBytesBufIt,
         class BlockType = default_block_t<>>
class bko_1_3_cnf_fss final {
public:
  static constexpr int domain_bits = DomainBits;
  static constexpr int range_bits = RangeBits;
  using domain_type = typename remove_cvref<DomainType>::type;
  using range_type = typename remove_cvref<RangeType>::type;
  using block_type = typename remove_cvref<BlockType>::type;
  using rand_perm_type = typename MsDpfPlusType::rand_perm_type;
  using rng_type = typename MsDpfPlusType::rng_type;

  static constexpr int range_type_bits = type_bits<range_type>();
  using nice_range_type = typename promote_unsigned<range_type>::type;

  PFSS_SST_STATIC_ASSERT(domain_bits > 0);
  PFSS_SST_STATIC_ASSERT(range_bits > 0);
  PFSS_SST_STATIC_ASSERT(is_unsigned_integer<range_type>::value);

  static constexpr std::size_t lambda = bko_1_3_security_parameter;
  // Specifies the number of block_type's output by the PRG.
  static constexpr std::size_t prg_output_size = 2;

  bko_1_3_cnf_fss() noexcept = default;
  bko_1_3_cnf_fss(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bko_1_3_cnf_fss() noexcept = default;
  bko_1_3_cnf_fss(bko_1_3_cnf_fss const &) noexcept = default;
  bko_1_3_cnf_fss(bko_1_3_cnf_fss &&) noexcept = default;
  bko_1_3_cnf_fss &
  operator=(bko_1_3_cnf_fss const &) noexcept = default;
  bko_1_3_cnf_fss & operator=(bko_1_3_cnf_fss &&) noexcept = default;

  constexpr std::size_t key_blob_size() noexcept {
    return key_type().size();
  }

  // TODO: the number of bytes needed for buffered rng() is non-deterministic
  // due to find_cw's algorithm. How should we deal with that?
  //
  // This function calculates the buffer size of rng() based on the number
  // of calls made to rng(). It assumes that we enter the kth range bit == 0
  // block for initializing the control + sibling control bits (4 calls to
  // rng() made in that block), two calls to rng() for each domain bit to
  // initialize the random seeds p and q, and 2 calls to rng() in find_cw().
  std::size_t rand_buf_size() noexcept {
    return range_bits
               * (block_type().size() * (4 + domain_bits * 2 + 2))
           + MsDpfPlusType::rand_buf_size();
  }

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {5};
  }

  // inner_key_type defines the structure that holds the kth subkey where
  // k is in the range [0, range_bits).
  struct inner_key_type final {
    // Seeds for the PRG.
    block_type x_0;
    block_type y_0;
    block_type z_0;

    // CNF-shared sibling control bits and control bits.
    typename MsDpfPlusType::range_type b_0;
    typename MsDpfPlusType::range_type b_0_party_to_right;
    typename MsDpfPlusType::range_type c_0;
    typename MsDpfPlusType::range_type c_0_party_to_right;

    // Correction bits, one for each level.
    std::vector<typename MsDpfPlusType::range_type> r;
    std::vector<typename MsDpfPlusType::range_type> s;
    std::vector<typename MsDpfPlusType::range_type> t;
    std::vector<typename MsDpfPlusType::range_type> u;

    // 2*Lambda-bit-wide MS-DPF+ secret range values for
    // the root node.
    typename MsDpfPlusType::range_type v_L;
    typename MsDpfPlusType::range_type v_R;

    // Containers for MS-DPF+ gen keys for each level.
    std::vector<typename MsDpfPlusType::key_type> kappa;
    std::vector<typename MsDpfPlusType::key_type> kappahat;

    // prg_convert_for_power_of_two_groups() is parameterized by
    // two lambda-bit values, x and s. convertPRG_index corresponds
    // to s.
    block_type convertPRG_index;

    // Final correction word.
    nice_range_type w;

    inner_key_type()
        : b_0(1),
          b_0_party_to_right(1),
          c_0(1),
          c_0_party_to_right(1),
          v_L(lambda),
          v_R(lambda) {
    }

    std::size_t size() noexcept {
      using T = promote_unsigned<std::size_t>::type;
      std::size_t ms_dpf_plus_key_sizes = 0;

      for (int l = 1; l <= domain_bits; l++) {
        if ((l - 1) > 0) {
          ms_dpf_plus_key_sizes +=
              typename MsDpfPlusType::key_type(l - 1, lambda * 2)
                  .size();
        }
        ms_dpf_plus_key_sizes +=
            typename MsDpfPlusType::key_type(l, lambda).size();
      }

      // clang-format off
      return static_cast<std::size_t>(       
          // The prg seeds x_0, y_0, z_0.
          3 * block_type().size() +

          // The control bits b_0(_party_to_right) and c_0(_party_to_right).
          T(4) +

          // The correction bits, r, s, t and u.
          T(domain_bits * 4) +

          // The first MS-DPF+ range values, v_L and v_R.
          T(2* (lambda / uchar_bits + (lambda % uchar_bits ? 1 : 0))) +

          // The total size in bytes of the MS-DPF+ keys.
          T(ms_dpf_plus_key_sizes) +

          // The final correction word, convertPRG_index.
          block_type().size() +

          // The kth correction word
          sizeof(nice_range_type)
      );
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      out = x_0.serialize(out);
      out = y_0.serialize(out);
      out = z_0.serialize(out);
      out = b_0.serialize(out);
      out = b_0_party_to_right.serialize(out);
      out = c_0.serialize(out);
      out = c_0_party_to_right.serialize(out);
      out = v_L.serialize(out);
      out = v_R.serialize(out);

      for (int i = 0; i < domain_bits; i++) {
        out = r[i].serialize(out);
        out = s[i].serialize(out);
        out = t[i].serialize(out);
        out = u[i].serialize(out);
      }

      for (std::size_t l = 1; l <= domain_bits; l++) {
        if ((l - 1) > 0) {
          out = kappa[l - 2].serialize(out);
        }
        out = kappahat[l - 1].serialize(out);
      }
      out = convertPRG_index.serialize(out);
      out = serialize_integer(sizeof(nice_range_type) * uchar_bits,
                              w,
                              out);
      return out;
    }

    template<class InputIt>
    InputIt parse(InputIt in) {
      in = x_0.parse(in);
      in = y_0.parse(in);
      in = z_0.parse(in);
      in = b_0.parse(in);
      in = b_0_party_to_right.parse(in);
      in = c_0.parse(in);
      in = c_0_party_to_right.parse(in);
      in = v_L.parse(in);
      in = v_R.parse(in);
      for (int i = 0; i < domain_bits; i++) {
        typename MsDpfPlusType::range_type r_temp(1);
        typename MsDpfPlusType::range_type s_temp(1);
        typename MsDpfPlusType::range_type t_temp(1);
        typename MsDpfPlusType::range_type u_temp(1);

        in = r_temp.parse(in);
        in = s_temp.parse(in);
        in = t_temp.parse(in);
        in = u_temp.parse(in);
        r.push_back(r_temp);
        s.push_back(s_temp);
        t.push_back(t_temp);
        u.push_back(u_temp);
      }

      for (std::size_t l = 1; l <= domain_bits; l++) {
        if ((l - 1) > 0) {
          typename MsDpfPlusType::key_type kappa_temp(l - 1,
                                                      lambda * 2);
          in = kappa_temp.parse(in);
          kappa.push_back(kappa_temp);
        }
        typename MsDpfPlusType::key_type kappahat_temp(l, lambda);
        in = kappahat_temp.parse(in);
        kappahat.push_back(kappahat_temp);
      }
      in = convertPRG_index.parse(in);
      in = deserialize_integer(sizeof(nice_range_type) * uchar_bits,
                               w,
                               in);

      return in;
    }
  };

  // The key_type struct represends a CNF-FSS key that
  // gets dealt to a party.
  struct key_type final {

    static constexpr int domain_bits = DomainBits;
    static constexpr int range_bits = RangeBits;
    uint8_t party = 0;
    std::array<inner_key_type, range_bits> inner_keys;

    std::size_t size() noexcept {
      using T = promote_unsigned<std::size_t>::type;

      // clang-format off
      return static_cast<std::size_t>(
          // The header.
          T(key_header().size()) +
          
          // The party index value.
          T(1) +

          // The domain_bits value.
          T(1) +

          // The range_bits value.
          T(1) +

          // The inner key type sizes.
          T(inner_key_type().size() * range_bits)

      );
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      auto const h = key_header();
      out = std::copy(h.cbegin(), h.cend(), out);
      out = serialize_integer(uchar_bits, domain_bits, out);
      out = serialize_integer(uchar_bits, range_bits, out);
      out = serialize_integer(uchar_bits, party, out);
      for (auto & inner_key : inner_keys) {
        out = inner_key.serialize(out);
      }
      return out;
    }

    // Assumes that header, domain_bits, and range_bits have already
    // been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {
      std::advance(in, key_header().size());
      unsigned char const db = *in++;

      if (unsigned_ne(db, domain_bits)) {
        throw std::invalid_argument("key blob is corrupt");
      }
      unsigned char const rb = *in++;
      if (unsigned_ne(rb, range_bits)) {
        throw std::invalid_argument("key blob is corrupt");
      }
      unsigned char const pt = *in++;
      if (pt > 2) {
        throw std::invalid_argument("key blob is corrupt");
      }
      party = static_cast<uint8_t>(pt);
      for (unsigned char i = 0; i < rb; i++) {
        in = inner_keys[i].parse(in);
      }
      return in;
    }

    inner_key_type & operator[](std::size_t const i) {
      assert(i < inner_keys.size());
      return inner_keys[i];
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

  static void gen(std::array<key_type, 3> & key,
                  domain_type const & domain_slot,
                  range_type const & range_value,
                  rand_perm_type & pi,
                  rng_type & rng,
                  RandBytesBufIt control_bits_rand_buf_it,
                  RandBytesBufIt ms_dpf_plus_rand_buf_it) noexcept {
    // Set the party indices of the keys.
    for (std::size_t i = 0; i < 3; i++) {
      key[i].party = i;
    }

    // Initialize the variable used to store the output of
    // the PRG, G.
    block_type prg_output[prg_output_size];

    // Initialize variables that will be used for XOR-ing
    // with other MS-DPF+ range types.
    typename MsDpfPlusType::range_type z2n_one(1);
    z2n_one[0] = 1;

    typename MsDpfPlusType::range_type z2n_zero(1);
    typename MsDpfPlusType::range_type z2lambda_zero(lambda);

    // Initialize storage for control bits and sibling control
    // bits.
    typename MsDpfPlusType::range_type c_0[3] = {z2n_zero,
                                                 z2n_zero,
                                                 z2n_zero};
    typename MsDpfPlusType::range_type b_0[3] = {z2n_zero,
                                                 z2n_zero,
                                                 z2n_zero};

    // Loop through each bit of the secret range value.
    for (int k = 0; k < range_bits; k++) {
      // The XOR sum of all sibling control bits will always
      // need to equal zero, according to the invariant
      // described in BKO21. This is because in gen, we are
      // on an on-path node with the sibling node being off-path;
      // or we are traversing a tree where all nodes are off-path.
      // We use the first case when the kth range bit == 1, and
      // the second case when the kth range bit == 0.
      b_0[0][0] = *control_bits_rand_buf_it++;
      b_0[1][0] = *control_bits_rand_buf_it++;
      b_0[2] = b_0[0] ^ b_0[1];
      b_0[0][0] = static_cast<unsigned char>(b_0[0][0] & 1);
      b_0[1][0] = static_cast<unsigned char>(b_0[1][0] & 1);
      b_0[2][0] = static_cast<unsigned char>(b_0[2][0] & 1);

      // The following if-else block corresponds to the control
      // bit and seed invariants described in BKO21. The variables
      // set serve as the root-level seeds and control bits.
      c_0[0][0] = *control_bits_rand_buf_it++;
      c_0[1][0] = *control_bits_rand_buf_it++;
      block_type seed_A = rng();
      block_type seed_B = rng();
      block_type seed_C = rng();
      zero_msbs(2, seed_A);
      zero_msbs(2, seed_B);
      zero_msbs(2, seed_C);
      if (getbitx(range_value, k) == 0) {
        c_0[2] = c_0[0] ^ c_0[1];

        key[0][k].x_0 = seed_A;
        key[0][k].y_0 = seed_B;
        key[0][k].z_0 = seed_C;

        key[1][k].x_0 = seed_B;
        key[1][k].y_0 = seed_C;
        key[1][k].z_0 = seed_A;

        key[2][k].x_0 = seed_C;
        key[2][k].y_0 = seed_A;
        key[2][k].z_0 = seed_B;
      } else {
        c_0[2] = ~(c_0[0] ^ c_0[1]);

        block_type seed_D = rng();
        zero_msbs(2, seed_D);

        key[0][k].x_0 = seed_A;
        key[0][k].y_0 = seed_D;
        key[0][k].z_0 = seed_B;

        key[1][k].x_0 = seed_B;
        key[1][k].y_0 = seed_D;
        key[1][k].z_0 = seed_C;

        key[2][k].x_0 = seed_C;
        key[2][k].y_0 = seed_D;
        key[2][k].z_0 = seed_A;
      }
      c_0[0][0] = static_cast<unsigned char>(c_0[0][0] & 1);
      c_0[1][0] = static_cast<unsigned char>(c_0[1][0] & 1);
      c_0[2][0] = static_cast<unsigned char>(c_0[2][0] & 1);

      // Define the variables to store the seeds and control
      // bits for each level. These variables will get updated
      // for each iteration through the domain bits.
      block_type x_P[3];
      block_type y_P[3];
      block_type z_P[3];
      typename MsDpfPlusType::range_type b_P[3] = {z2n_zero,
                                                   z2n_zero,
                                                   z2n_zero};
      typename MsDpfPlusType::range_type c_P[3] = {z2n_zero,
                                                   z2n_zero,
                                                   z2n_zero};

      // Initialize the values of the variables above to the
      // root-level values.
      for (std::size_t party = 0; party < 3; party++) {
        x_P[party] = key[party][k].x_0;
        y_P[party] = key[party][k].y_0;
        z_P[party] = key[party][k].z_0;
        b_P[party] = b_0[party];
        c_P[party] = c_0[party];

        key[party][k].b_0 = b_0[party];
        key[party][k].b_0_party_to_right =
            b_0[party_to_the_right(party)];
        key[party][k].c_0 = c_0[party];
        key[party][k].c_0_party_to_right =
            c_0[party_to_the_right(party)];
      }

      // Iterate through each bit of the secret domain location.
      for (std::size_t l = 1; l <= domain_bits; l++) {
        bool const alpha_i = getbitx(domain_slot, l - 1);

        // Get the l and the l - 1 LSB's of the secret location.
        domain_type alpha_l = static_cast<domain_type>(
            get_mask<domain_type>(l) & domain_slot);
        domain_type alpha_l_minus_one = static_cast<domain_type>(
            get_mask<domain_type>(l - 1) & domain_slot);

        // Define the variables used to hold each portion of the PRG's (G's) output.
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_G_x = {
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda,
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_H_x = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_Hhat_x = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};

        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_G_y = {
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda,
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_H_y = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_Hhat_y = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};

        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_G_z = {
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda,
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_H_z = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_Hhat_z = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};

        for (std::size_t party = 0; party < 3; party++) {
          // Zero out the two MSB's of the seeds because block_type consists of 128 bit
          // blocks while lambda is currently 126 bits.
          zero_msbs(x_P[party].bits - lambda, x_P[party]);
          zero_msbs(y_P[party].bits - lambda, y_P[party]);
          zero_msbs(z_P[party].bits - lambda, z_P[party]);

          // Apply the PRG to the seeds and break up the PRG output into the different components.
          prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                           x_P[party],
                                                           prg_output);
          decompose_prg_output<block_type,
                               typename MsDpfPlusType::range_type,
                               prg_output_size>(
              prg_output,
              decomp_G_x[0][party],
              decomp_G_x[1][party],
              decomp_H_x[0][party],
              decomp_Hhat_x[0][party],
              decomp_H_x[1][party],
              decomp_Hhat_x[1][party]);

          prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                           y_P[party],
                                                           prg_output);
          decompose_prg_output<block_type,
                               typename MsDpfPlusType::range_type,
                               prg_output_size>(
              prg_output,
              decomp_G_y[0][party],
              decomp_G_y[1][party],
              decomp_H_y[0][party],
              decomp_Hhat_y[0][party],
              decomp_H_y[1][party],
              decomp_Hhat_y[1][party]);

          prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                           z_P[party],
                                                           prg_output);
          decompose_prg_output<block_type,
                               typename MsDpfPlusType::range_type,
                               prg_output_size>(
              prg_output,
              decomp_G_z[0][party],
              decomp_G_z[1][party],
              decomp_H_z[0][party],
              decomp_Hhat_z[0][party],
              decomp_H_z[1][party],
              decomp_Hhat_z[1][party]);
        }
        // Define variables to hold the secret MS-DPF+ range values.
        std::array<std::vector<typename MsDpfPlusType::range_type>, 2>
            v = {ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                     .vector_lambda,
                 ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                     .vector_lambda};
        std::vector<typename MsDpfPlusType::range_type> v_l =
            ms_dpf_plus_range_type_vector_two_lambda<MsDpfPlusType>()
                .vector_two_lambda;
        std::vector<typename MsDpfPlusType::range_type> vhat_l =
            ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                .vector_lambda;

        typename MsDpfPlusType::range_type p_range(lambda);
        construct_random_ms_dpf_plus_range_type<block_type>(rng,
                                                            p_range);

        typename MsDpfPlusType::range_type q_range(lambda);
        construct_random_ms_dpf_plus_range_type<block_type>(rng,
                                                            q_range);

        // Set the MS-DPF+ range values for f.
        // If the kth range bit is a 1, we calculate the MS-DPF+
        // secret values according to equations 7 and 8 in BKO21.
        // Otherwise, we set all of the MS-DPF+ shares to the same
        // random value.
        // When the range bit is zero, we need all parties to have the
        // same MS-DPF+ range value. So follow equation 7 for the
        // first party and either use that for all parties if the range
        // bit is zero. Otherwise, we calculate the range values of the
        // other parties according to equation 7 of BKO21.
        const std::size_t stop_at =
            getbitx(range_value, k) == 0 ? 1 : 3;
        for (std::size_t party = 0; party < stop_at; party++) {
          std::size_t Q = party_to_the_left(party);

          v[0][party] = (alpha_i ^ 1) ?
                            (decomp_G_x[0][Q] + (p_range + q_range)) :
                            decomp_G_x[0][Q];
          v[1][party] = alpha_i ?
                            (decomp_G_x[1][Q] + (p_range + q_range)) :
                            decomp_G_x[1][Q];
          v_l[party] = v[0][party] || v[1][party];
        }
        if (getbitx(range_value, k) == 0) {
          for (std::size_t party = 1; party < 3; party++) {
            v[1][party] = v[1][0];
            v[0][party] = v[0][0];
            v_l[party] = v[0][party] || v[1][party];
          }
        }

        ms_dpf_plus_rand_buf_it = generate_ms_dpf_keys<MsDpfPlusType>(
            false,
            k,
            l - 1,
            bko_1_3_security_parameter * 2,
            alpha_l_minus_one,
            v_l,
            pi,
            rng,
            ms_dpf_plus_rand_buf_it,
            key);
        // If we are only on the first level of the binary tree,
        // store the MS-DPF+ range values in the clear.
        if (l - 1 == 0) {
          for (std::size_t party = 0; party < 3; party++) {
            key[party][k].v_L = v[0][party];
            key[party][k].v_R = v[1][party];
          }
        }

        // Calculate correction bits.
        typename MsDpfPlusType::range_type h_L =
            decomp_H_x[0][0] ^ decomp_H_x[0][1] ^ decomp_H_x[0][2]
            ^ decomp_H_y[0][0];
        typename MsDpfPlusType::range_type h_R =
            decomp_H_x[1][0] ^ decomp_H_x[1][1] ^ decomp_H_x[1][2]
            ^ decomp_H_y[1][0];
        typename MsDpfPlusType::range_type hhat_L =
            decomp_Hhat_x[0][0] ^ decomp_Hhat_x[0][1]
            ^ decomp_Hhat_x[0][2] ^ decomp_Hhat_y[0][0];
        typename MsDpfPlusType::range_type hhat_R =
            decomp_Hhat_x[1][0] ^ decomp_Hhat_x[1][1]
            ^ decomp_Hhat_x[1][2] ^ decomp_Hhat_y[1][0];

        // Store correction bits in the keys.
        if (alpha_i == 0) {
          for (std::size_t party = 0; party < 3; party++) {
            key[party][k].r.push_back(h_L);
            key[party][k].s.push_back(z2n_one ^ h_R);
            key[party][k].t.push_back(z2n_one ^ hhat_L);
            key[party][k].u.push_back(hhat_R);
          }
        } else {
          for (std::size_t party = 0; party < 3; party++) {
            key[party][k].r.push_back(z2n_one ^ h_L);
            key[party][k].s.push_back(h_R);
            key[party][k].t.push_back(hhat_L);
            key[party][k].u.push_back(z2n_one ^ hhat_R);
          }
        }

        // Update the control bits for the next level.
        for (std::size_t party = 0; party < 3; party++) {
          if (alpha_i == 0) {
            b_P[party] =
                (c_P[party][0] ? key[party][k].r.back() : z2n_zero)
                ^ decomp_H_x[0][party] ^ decomp_H_y[0][party];
            c_P[party] =
                (c_P[party][0] ? key[party][k].t.back() : z2n_zero)
                ^ decomp_Hhat_x[0][party] ^ decomp_Hhat_y[0][party];
          } else {
            b_P[party] =
                (c_P[party][0] ? key[party][k].s.back() : z2n_zero)
                ^ decomp_H_x[1][party] ^ decomp_H_y[1][party];
            c_P[party] =
                (c_P[party][0] ? key[party][k].u.back() : z2n_zero)
                ^ decomp_Hhat_x[1][party] ^ decomp_Hhat_y[1][party];
          }
        }

        // Use the updated control bits to calculate the MS-DPF+ range
        // value vhat_l. If the kth range bit is zero, set each of the
        // MS-DPF+ range value shares to the same random value, as we
        // did for vhat_l_minus_one.
        if (getbitx(range_value, k) != 0) {
          for (std::size_t party = 0; party < 3; party++) {
            if (b_P[party_to_the_left(party)][0] == z2n_zero[0]) {
              vhat_l[party] = p_range;
            } else {
              vhat_l[party] = q_range;
            }
          }
        } else {
          typename MsDpfPlusType::range_type temp(lambda);
          for (std::size_t i = 0; i < temp.size(); i++) {
            temp[i] =
                static_cast<unsigned char>(*ms_dpf_plus_rand_buf_it++);
          }
          if ((lambda % uchar_bits) != 0) {
            temp[temp.size() - 1] &= get_mask<unsigned char>(
                uchar_bits - (lambda % uchar_bits));
          }
          for (std::size_t party = 0; party < 3; party++) {
            vhat_l[party] = temp;
          }
        }

        // Generate and store the MS-DPF+ keys.
        std::vector<typename MsDpfPlusType::key_type> kappahat_P(
            3,
            typename MsDpfPlusType::key_type(l, lambda));
        ms_dpf_plus_rand_buf_it = generate_ms_dpf_keys<MsDpfPlusType>(
            true,
            k,
            l,
            bko_1_3_security_parameter,
            alpha_l,
            vhat_l,
            pi,
            rng,
            ms_dpf_plus_rand_buf_it,
            key);

        // Update the seeds for the next level, preserving the invariant
        // described in equation 2 from BKO21.
        for (std::size_t party = 0; party < 3; party++) {
          if (b_P[party][0] != b_P[party_to_the_right(party)][0]) {
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_y[alpha_i][party]
                    ^ (b_P[party][0] ? decomp_G_y[alpha_i][party]
                                           ^ v[alpha_i][party] :
                                       z2lambda_zero)
                    ^ vhat_l[party],
                x_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_x[alpha_i][party]
                    ^ (b_P[party][0] ?
                           decomp_G_x[alpha_i][party]
                               ^ decomp_G_z[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                y_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                v[alpha_i][party]
                    ^ (b_P[party][0] ?
                           v[alpha_i][party]
                               ^ decomp_G_y[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                z_P[party]);
          } else {
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_x[alpha_i][party]
                    ^ (b_P[party][0] ?
                           decomp_G_x[alpha_i][party]
                               ^ decomp_G_z[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                x_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_y[alpha_i][party]
                    ^ (b_P[party][0] ? decomp_G_y[alpha_i][party]
                                           ^ v[alpha_i][party] :
                                       z2lambda_zero)
                    ^ vhat_l[party],
                y_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_z[alpha_i][party]
                    ^ (b_P[party][0] ?
                           decomp_G_z[alpha_i][party]
                               ^ decomp_G_x[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                z_P[party]);
          }
        }
      }

      // Compute the final correction word.
      if (getbitx(range_value, k) == 0) {
        // Sample the prg_convert input parameters at random.
        block_type temp_s = rng();
        block_type temp_w = rng();
        zero_msbs(temp_s.bits - lambda, temp_s);
        zero_msbs(temp_w.bits - lambda, temp_w);
        for (std::size_t party = 0; party < 3; party++) {
          key[party][k].convertPRG_index = temp_s;
          key[party][k].w =
              block_to_range_type<block_type, nice_range_type>(
                  range_bits,
                  temp_w);
        }
      } else {
        // Calculate the prg_convert input parameters subject
        // to the constraints described in equations 11, 12,
        // and Appendix A.4 of BKO21.
        block_type convertPRG_index;
        nice_range_type w;
        find_cw(range_bits,
                k,
                lambda,
                pi,
                rng,
                x_P,
                y_P,
                convertPRG_index,
                w);
        for (std::size_t party = 0; party < 3; party++) {
          key[party][k].convertPRG_index = convertPRG_index;
          key[party][k].w = w;
          zero_msbs(key[party][k].convertPRG_index.bits - lambda,
                    key[party][k].convertPRG_index);
        }
      }
    }
  }

  static void eval(key_type & key,
                   domain_type const & x,
                   rand_perm_type & pi,
                   std::array<range_type, 2> & result) noexcept {
    // Initialize variables that will be used for XOR-ing
    // with other MS-DPF+ range types.
    typename MsDpfPlusType::range_type z2n_one(1);
    z2n_one[0] = 1;

    typename MsDpfPlusType::range_type z2n_zero(1);
    z2n_zero[0] = 0;

    typename MsDpfPlusType::range_type z2lambda_zero(lambda);

    // Define the variables that will be iteratively updated
    // to contain each leve's seed, control bit, and sibling
    // control bit values.
    block_type x_P;
    block_type y_P;
    block_type z_P;
    block_type prg_output[prg_output_size];
    typename MsDpfPlusType::range_type b_P = z2n_zero;
    typename MsDpfPlusType::range_type bbar_P = z2n_zero;
    typename MsDpfPlusType::range_type c_P = z2n_zero;
    typename MsDpfPlusType::range_type cbar_P = z2n_zero;

    // Define the variables that will hold each level's MS-DPF+
    // eval results.
    std::array<typename MsDpfPlusType::range_type, 2> v =
        ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
            .array_lambda_two_d;
    typename MsDpfPlusType::range_type vhat_P(lambda);
    typename MsDpfPlusType::range_type v_P(lambda * 2);

    // Iterate through each k range bit.
    for (int k = 0; k < range_bits; k++) {
      // Initially set each seed, control bits, and sibling control bits
      // to the initial values stored in the key.
      x_P = key[k].x_0;
      y_P = key[k].y_0;
      z_P = key[k].z_0;
      b_P = key[k].b_0;
      bbar_P = key[k].b_0_party_to_right;
      c_P = key[k].c_0;
      cbar_P = key[k].c_0_party_to_right;

      // Iterate through each domain bit.
      for (std::size_t l = 1; l <= domain_bits; l++) {
        // Get the l and l - minus one LSB's of alpha.
        domain_type alpha_l =
            static_cast<domain_type>(get_mask<domain_type>(l) & x);
        domain_type alpha_l_minus_one =
            static_cast<domain_type>(get_mask<domain_type>(l - 1) & x);

        if (l - 1 > 0) {
          MsDpfPlusType MsDpfPlus(l - 1, lambda * 2);
          v_P = MsDpfPlus.eval(key[k].kappa[l - 2],
                               alpha_l_minus_one,
                               pi);
          split_ms_dpf_plus_range_type(v_P, v[0], v[1]);
        } else {
          v[0] = key[k].v_L;
          v[1] = key[k].v_R;
        }

        // Define the variables that will hold the result of plugging
        // in the seeds into the PRG, G.
        std::array<typename MsDpfPlusType::range_type, 2> decomp_G_x =
            ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                .array_lambda_two_d;
        std::array<typename MsDpfPlusType::range_type, 2> decomp_H_x =
            ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                .array_one_two_d;
        std::array<typename MsDpfPlusType::range_type, 2>
            decomp_Hhat_x =
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one_two_d;

        std::array<typename MsDpfPlusType::range_type, 2> decomp_G_y =
            ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                .array_lambda_two_d;
        std::array<typename MsDpfPlusType::range_type, 2> decomp_H_y =
            ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                .array_one_two_d;
        std::array<typename MsDpfPlusType::range_type, 2>
            decomp_Hhat_y =
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one_two_d;

        std::array<typename MsDpfPlusType::range_type, 2> decomp_G_z =
            ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                .array_lambda_two_d;
        std::array<typename MsDpfPlusType::range_type, 2> decomp_H_z =
            ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                .array_one_two_d;
        std::array<typename MsDpfPlusType::range_type, 2>
            decomp_Hhat_z =
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one_two_d;

        zero_msbs(x_P.bits - lambda, x_P);
        zero_msbs(y_P.bits - lambda, y_P);
        zero_msbs(z_P.bits - lambda, z_P);

        prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                         x_P,
                                                         prg_output);
        decompose_prg_output<block_type,
                             typename MsDpfPlusType::range_type,
                             prg_output_size>(prg_output,
                                              decomp_G_x[0],
                                              decomp_G_x[1],
                                              decomp_H_x[0],
                                              decomp_Hhat_x[0],
                                              decomp_H_x[1],
                                              decomp_Hhat_x[1]);

        prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                         y_P,
                                                         prg_output);
        decompose_prg_output<block_type,
                             typename MsDpfPlusType::range_type,
                             prg_output_size>(prg_output,
                                              decomp_G_y[0],
                                              decomp_G_y[1],
                                              decomp_H_y[0],
                                              decomp_Hhat_y[0],
                                              decomp_H_y[1],
                                              decomp_Hhat_y[1]);

        prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                         z_P,
                                                         prg_output);
        decompose_prg_output<block_type,
                             typename MsDpfPlusType::range_type,
                             prg_output_size>(prg_output,
                                              decomp_G_z[0],
                                              decomp_G_z[1],
                                              decomp_H_z[0],
                                              decomp_Hhat_z[0],
                                              decomp_H_z[1],
                                              decomp_Hhat_z[1]);

        // Update the values of the control bits and sibling control bits.
        bool const x_i = getbitx(x, l - 1);
        if (!x_i) {
          b_P = (key[k].r[l - 1][0] ? c_P : z2n_zero) ^ decomp_H_x[0]
                ^ decomp_H_y[0];
          bbar_P = (key[k].r[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_H_y[0] ^ decomp_H_z[0];
          c_P = (key[k].t[l - 1][0] ? c_P : z2n_zero) ^ decomp_Hhat_x[0]
                ^ decomp_Hhat_y[0];
          cbar_P = (key[k].t[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_Hhat_y[0] ^ decomp_Hhat_z[0];
        } else {
          b_P = (key[k].s[l - 1][0] ? c_P : z2n_zero) ^ decomp_H_x[1]
                ^ decomp_H_y[1];
          bbar_P = (key[k].s[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_H_y[1] ^ decomp_H_z[1];
          c_P = (key[k].u[l - 1][0] ? c_P : z2n_zero) ^ decomp_Hhat_x[1]
                ^ decomp_Hhat_y[1];
          cbar_P = (key[k].u[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_Hhat_y[1] ^ decomp_Hhat_z[1];
        }

        MsDpfPlusType MsDpfPlusHat(l, lambda);
        vhat_P = MsDpfPlusHat.eval(key[k].kappahat[l - 1], alpha_l, pi);

        // Update the values of the seeds for the next level.
        if (b_P[0] != bbar_P[0]) {
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_y[x_i]
                  ^ (b_P[0] ? decomp_G_y[x_i] ^ v[x_i] : z2lambda_zero)
                  ^ vhat_P,
              x_P);
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_x[x_i]
                  ^ (b_P[0] ? decomp_G_x[x_i] ^ decomp_G_z[x_i] :
                              z2lambda_zero)
                  ^ vhat_P,
              y_P);
          copy_ms_dpf_plus_range_type_to_block(
              v[x_i]
                  ^ (b_P[0] ? v[x_i] ^ decomp_G_y[x_i] : z2lambda_zero)
                  ^ vhat_P,
              z_P);
        } else {
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_x[x_i]
                  ^ (b_P[0] ? decomp_G_x[x_i] ^ decomp_G_z[x_i] :
                              z2lambda_zero)
                  ^ vhat_P,
              x_P);
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_y[x_i]
                  ^ (b_P[0] ? decomp_G_y[x_i] ^ v[x_i] : z2lambda_zero)
                  ^ vhat_P,
              y_P);
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_z[x_i]
                  ^ (b_P[0] ? decomp_G_z[x_i] ^ decomp_G_x[x_i] :
                              z2lambda_zero)
                  ^ vhat_P,
              z_P);
        }
      }

      // Calculate the CNF-shared eval results based on the methods
      // described in appendix A.4 of BKO21.
      zero_msbs(x_P.bits - lambda, x_P);
      zero_msbs(y_P.bits - lambda, y_P);
      zero_msbs(z_P.bits - lambda, z_P);
      const nice_range_type m = static_cast<nice_range_type>(1)
                                << (range_bits - k);
      const nice_range_type ghat_x =
          prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
              range_bits,
              range_bits - k,
              pi,
              x_P,
              key[k].convertPRG_index);
      const nice_range_type ghat_y =
          prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
              range_bits,
              range_bits - k,
              pi,
              y_P,
              key[k].convertPRG_index);
      const nice_range_type ghat_z =
          prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
              range_bits,
              range_bits - k,
              pi,
              z_P,
              key[k].convertPRG_index);
      uint64_t rhs =
          (((key[k].w * ghat_x % m) - (key[k].w * ghat_y % m)) % m);
      result[0] += rhs * (static_cast<uint64_t>(1) << (k));

      rhs = (((key[k].w * ghat_z % m) - (key[k].w * ghat_y % m)) % m);
      result[1] += rhs * (static_cast<uint64_t>(1) << (k));
    }
  }
};

//
// Runtime domain_bits and range_bits.
//
template<class DomainType,
         class RangeType,
         class MsDpfPlusType,
         class RandBytesBufIt,
         class BlockType = default_block_t<>>
class bko_1_3_cnf_fss_v final {
public:
  int const domain_bits;
  int const range_bits;
  using domain_type = typename remove_cvref<DomainType>::type;
  using range_type = typename remove_cvref<RangeType>::type;
  using block_type = typename remove_cvref<BlockType>::type;
  using rand_perm_type = typename MsDpfPlusType::rand_perm_type;
  using rng_type = typename MsDpfPlusType::rng_type;

  static constexpr int range_type_bits = type_bits<range_type>();
  using nice_range_type = typename promote_unsigned<range_type>::type;

  PFSS_SST_STATIC_ASSERT(is_unsigned_integer<range_type>::value);
  static constexpr std::size_t lambda = bko_1_3_security_parameter;
  static constexpr std::size_t prg_output_size = 2;

  bko_1_3_cnf_fss_v() noexcept = default;
  bko_1_3_cnf_fss_v(int const domain_bits,
                    int const range_bits) noexcept
      : domain_bits(domain_bits),
        range_bits(range_bits) {
    assert(domain_bits > 0);
    assert(range_bits > 0);
  }

  ~bko_1_3_cnf_fss_v() noexcept = default;
  bko_1_3_cnf_fss_v(bko_1_3_cnf_fss_v const &) noexcept = default;
  bko_1_3_cnf_fss_v(bko_1_3_cnf_fss_v &&) noexcept = default;
  bko_1_3_cnf_fss_v &
  operator=(bko_1_3_cnf_fss_v const &) noexcept = default;
  bko_1_3_cnf_fss_v &
  operator=(bko_1_3_cnf_fss_v &&) noexcept = default;

  constexpr std::size_t key_blob_size() noexcept {
    return key_type().size();
  }

  // TODO: the number of bytes needed for buffered rng() is non-deterministic
  // due to find_cw's algorithm. How should we deal with that?
  //
  // This function calculates the buffer size of rng() based on the number
  // of calls made to rng(). It assumes that we enter the kth range bit == 0
  // block for initializing the control + sibling control bits (4 calls to
  // rng() made in that block), two calls to rng() for each domain bit to
  // initialize the random seeds p and q, and 2 calls to rng() in find_cw().
  std::size_t rand_buf_size() noexcept {
    return range_bits
               * (block_type().size() * (4 + domain_bits * 2 + 2))
           + MsDpfPlusType::rand_buf_size();
  }

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {5};
  }

  struct inner_key_type final {
    block_type x_0;
    block_type y_0;
    block_type z_0;

    typename MsDpfPlusType::range_type b_0;
    typename MsDpfPlusType::range_type b_0_party_to_right;
    typename MsDpfPlusType::range_type c_0;
    typename MsDpfPlusType::range_type c_0_party_to_right;

    std::vector<typename MsDpfPlusType::range_type> r;
    std::vector<typename MsDpfPlusType::range_type> s;
    std::vector<typename MsDpfPlusType::range_type> t;
    std::vector<typename MsDpfPlusType::range_type> u;

    typename MsDpfPlusType::range_type v_L;
    typename MsDpfPlusType::range_type v_R;
    std::vector<typename MsDpfPlusType::key_type> kappa;
    std::vector<typename MsDpfPlusType::key_type> kappahat;

    block_type convertPRG_index;
    nice_range_type w;

    inner_key_type()
        : b_0(1),
          b_0_party_to_right(1),
          c_0(1),
          c_0_party_to_right(1),
          v_L(lambda),
          v_R(lambda) {
    }

    std::size_t size(const int domain_bits) noexcept {
      using T = promote_unsigned<std::size_t>::type;
      std::size_t ms_dpf_plus_key_sizes = 0;

      for (int l = 1; l <= domain_bits; l++) {
        if ((l - 1) > 0) {
          ms_dpf_plus_key_sizes +=
              typename MsDpfPlusType::key_type(l - 1, lambda * 2)
                  .size();
        }
        ms_dpf_plus_key_sizes +=
            typename MsDpfPlusType::key_type(l, lambda).size();
      }

      // clang-format off
      return static_cast<std::size_t>(       
          // The prg seeds x_0, y_0, z_0.
          3 * block_type().size() +

          // The control bits b_0(_party_to_right) and c_0(_party_to_right).
          T(4) +

          // The correction bits, r, s, t and u.
          T(domain_bits * 4) +

          // The first MS-DPF+ range values, v_L and v_R.
          T(2* (lambda / uchar_bits + (lambda % uchar_bits ? 1 : 0))) +

          // The total size in bytes of the MS-DPF+ keys.
          T(ms_dpf_plus_key_sizes) +

          // The final correction word, convertPRG_index.
          block_type().size() +

          // The kth correction word
          sizeof(nice_range_type)
      );
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(int const & domain_bits, OutputIt out) const {
      out = x_0.serialize(out);
      out = y_0.serialize(out);
      out = z_0.serialize(out);
      out = b_0.serialize(out);
      out = b_0_party_to_right.serialize(out);
      out = c_0.serialize(out);
      out = c_0_party_to_right.serialize(out);
      out = v_L.serialize(out);
      out = v_R.serialize(out);

      for (int i = 0; i < domain_bits; i++) {
        out = r[i].serialize(out);
        out = s[i].serialize(out);
        out = t[i].serialize(out);
        out = u[i].serialize(out);
      }

      for (int l = 1; l <= domain_bits; l++) {
        if ((l - 1) > 0) {
          out = kappa[l - 2].serialize(out);
        }
        out = kappahat[l - 1].serialize(out);
      }
      out = convertPRG_index.serialize(out);
      out = serialize_integer(sizeof(nice_range_type) * uchar_bits,
                              w,
                              out);
      return out;
    }

    template<class InputIt>
    InputIt parse(int const & domain_bits, InputIt in) {
      in = x_0.parse(in);
      in = y_0.parse(in);
      in = z_0.parse(in);
      in = b_0.parse(in);
      in = b_0_party_to_right.parse(in);
      in = c_0.parse(in);
      in = c_0_party_to_right.parse(in);
      in = v_L.parse(in);
      in = v_R.parse(in);
      for (int i = 0; i < domain_bits; i++) {
        typename MsDpfPlusType::range_type r_temp(1);
        typename MsDpfPlusType::range_type s_temp(1);
        typename MsDpfPlusType::range_type t_temp(1);
        typename MsDpfPlusType::range_type u_temp(1);

        in = r_temp.parse(in);
        in = s_temp.parse(in);
        in = t_temp.parse(in);
        in = u_temp.parse(in);
        r.push_back(r_temp);
        s.push_back(s_temp);
        t.push_back(t_temp);
        u.push_back(u_temp);
      }

      for (int l = 1; l <= domain_bits; l++) {
        if ((l - 1) > 0) {
          typename MsDpfPlusType::key_type kappa_temp(l - 1,
                                                      lambda * 2);
          in = kappa_temp.parse(in);
          kappa.push_back(kappa_temp);
        }
        typename MsDpfPlusType::key_type kappahat_temp(l, lambda);
        in = kappahat_temp.parse(in);
        kappahat.push_back(kappahat_temp);
      }
      in = convertPRG_index.parse(in);
      in = deserialize_integer(sizeof(nice_range_type) * uchar_bits,
                               w,
                               in);

      return in;
    }
  };

  struct key_type final {

    int domain_bits;
    int range_bits;
    uint8_t party = 0;
    std::vector<inner_key_type> inner_keys;

    key_type(int const domain_bits, int const range_bits)
        : domain_bits(domain_bits),
          range_bits(range_bits) {
      inner_keys.resize(range_bits);
    }

    std::size_t size() noexcept {
      using T = promote_unsigned<std::size_t>::type;

      // clang-format off
      return static_cast<std::size_t>(
          // The header.
          T(key_header().size()) +
          
          // The party index value.
          T(1) +

          // The domain_bits value.
          T(1) +

          // The range_bits value.
          T(1) +

          // The inner key type sizes.
          T(inner_key_type().size(domain_bits) * range_bits)

      );
      // clang-format on
    }

    template<class OutputIt>
    OutputIt serialize(OutputIt out) const {
      auto const h = key_header();
      out = std::copy(h.cbegin(), h.cend(), out);
      out = serialize_integer(uchar_bits, domain_bits, out);
      out = serialize_integer(uchar_bits, range_bits, out);
      out = serialize_integer(uchar_bits, party, out);
      for (auto & inner_key : inner_keys) {
        out = inner_key.serialize(domain_bits, out);
      }
      return out;
    }

    // Assumes that header, domain_bits, and range_bits have already
    // been parsed and verified, and that the blob size has also been
    // verified.
    template<class InputIt>
    InputIt parse(InputIt in) {
      std::advance(in, key_header().size());
      unsigned char const db = *in++;

      if (unsigned_ne(db, domain_bits)) {
        throw std::invalid_argument("key blob is corrupt");
      }
      unsigned char const rb = *in++;
      if (unsigned_ne(rb, range_bits)) {
        throw std::invalid_argument("key blob is corrupt");
      }
      unsigned char const pt = *in++;
      if (pt > 2) {
        throw std::invalid_argument("key blob is corrupt");
      }
      party = static_cast<uint8_t>(pt);
      for (unsigned char i = 0; i < rb; i++) {
        in = inner_keys[i].parse(domain_bits, in);
      }
      return in;
    }

    inner_key_type & operator[](std::size_t const i) {
      assert(i < inner_keys.size());
      return inner_keys[i];
    }
  };

  key_type make_key() noexcept {
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

  void gen(std::array<key_type, 3> & key,
           domain_type const & domain_slot,
           range_type const & range_value,
           rand_perm_type & pi,
           rng_type & rng,
           RandBytesBufIt control_bits_rand_buf_it,
           RandBytesBufIt ms_dpf_plus_rand_buf_it) noexcept {
    // Set the party indices of the keys.
    for (std::size_t i = 0; i < 3; i++) {
      key[i].party = i;
    }

    // Initialize the variable used to store the output of
    // the PRG, G.
    block_type prg_output[prg_output_size];

    // Initialize variables that will be used for XOR-ing
    // with other MS-DPF+ range types.
    typename MsDpfPlusType::range_type z2n_one(1);
    z2n_one[0] = 1;

    typename MsDpfPlusType::range_type z2n_zero(1);
    typename MsDpfPlusType::range_type z2lambda_zero(lambda);

    // Initialize storage for control bits and sibling control
    // bits.
    typename MsDpfPlusType::range_type c_0[3] = {z2n_zero,
                                                 z2n_zero,
                                                 z2n_zero};
    typename MsDpfPlusType::range_type b_0[3] = {z2n_zero,
                                                 z2n_zero,
                                                 z2n_zero};

    // Loop through each bit of the secret range value.
    for (int k = 0; k < range_bits; k++) {
      // The XOR sum of all sibling control bits will always
      // need to equal zero, according to the invariant
      // described in BKO21. This is because in gen, we are
      // on an on-path node with the sibling node being off-path;
      // or we are traversing a tree where all nodes are off-path.
      // We use the first case when the kth range bit == 1, and
      // the second case when the kth range bit == 0.
      b_0[0][0] = *control_bits_rand_buf_it++;
      b_0[1][0] = *control_bits_rand_buf_it++;
      b_0[2] = b_0[0] ^ b_0[1];
      b_0[0][0] = static_cast<unsigned char>(b_0[0][0] & 1);
      b_0[1][0] = static_cast<unsigned char>(b_0[1][0] & 1);
      b_0[2][0] = static_cast<unsigned char>(b_0[2][0] & 1);

      // The following if-else block corresponds to the control
      // bit and seed invariants described in BKO21. The variables
      // set serve as the root-level seeds and control bits.
      c_0[0][0] = *control_bits_rand_buf_it++;
      c_0[1][0] = *control_bits_rand_buf_it++;
      block_type seed_A = rng();
      block_type seed_B = rng();
      block_type seed_C = rng();
      zero_msbs(2, seed_A);
      zero_msbs(2, seed_B);
      zero_msbs(2, seed_C);
      if (getbitx(range_value, k) == 0) {
        c_0[2] = c_0[0] ^ c_0[1];

        key[0][k].x_0 = seed_A;
        key[0][k].y_0 = seed_B;
        key[0][k].z_0 = seed_C;

        key[1][k].x_0 = seed_B;
        key[1][k].y_0 = seed_C;
        key[1][k].z_0 = seed_A;

        key[2][k].x_0 = seed_C;
        key[2][k].y_0 = seed_A;
        key[2][k].z_0 = seed_B;
      } else {
        c_0[2] = ~(c_0[0] ^ c_0[1]);

        block_type seed_D = rng();
        zero_msbs(2, seed_D);

        key[0][k].x_0 = seed_A;
        key[0][k].y_0 = seed_D;
        key[0][k].z_0 = seed_B;

        key[1][k].x_0 = seed_B;
        key[1][k].y_0 = seed_D;
        key[1][k].z_0 = seed_C;

        key[2][k].x_0 = seed_C;
        key[2][k].y_0 = seed_D;
        key[2][k].z_0 = seed_A;
      }
      c_0[0][0] = static_cast<unsigned char>(c_0[0][0] & 1);
      c_0[1][0] = static_cast<unsigned char>(c_0[1][0] & 1);
      c_0[2][0] = static_cast<unsigned char>(c_0[2][0] & 1);

      // Define the variables to store the seeds and control
      // bits for each level. These variables will get updated
      // for each iteration through the domain bits.
      block_type x_P[3];
      block_type y_P[3];
      block_type z_P[3];
      typename MsDpfPlusType::range_type b_P[3] = {z2n_zero,
                                                   z2n_zero,
                                                   z2n_zero};
      typename MsDpfPlusType::range_type c_P[3] = {z2n_zero,
                                                   z2n_zero,
                                                   z2n_zero};

      // Initialize the values of the variables above to the
      // root-level values.
      for (std::size_t party = 0; party < 3; party++) {
        x_P[party] = key[party][k].x_0;
        y_P[party] = key[party][k].y_0;
        z_P[party] = key[party][k].z_0;
        b_P[party] = b_0[party];
        c_P[party] = c_0[party];

        key[party][k].b_0 = b_0[party];
        key[party][k].b_0_party_to_right =
            b_0[party_to_the_right(party)];
        key[party][k].c_0 = c_0[party];
        key[party][k].c_0_party_to_right =
            c_0[party_to_the_right(party)];
      }

      // Iterate through each bit of the secret domain location.
      for (int l = 1; l <= domain_bits; l++) {
        bool const alpha_i = getbitx(domain_slot, l - 1);

        // Get the l and the l - 1 LSB's of the secret location.
        domain_type alpha_l = static_cast<domain_type>(
            get_mask<domain_type>(l) & domain_slot);
        domain_type alpha_l_minus_one = static_cast<domain_type>(
            get_mask<domain_type>(l - 1) & domain_slot);

        // Define the variables used to hold each portion of the PRG's (G's) output.
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_G_x = {
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda,
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_H_x = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_Hhat_x = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};

        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_G_y = {
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda,
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_H_y = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_Hhat_y = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};

        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_G_z = {
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda,
                ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                    .array_lambda};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_H_z = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};
        std::array<std::array<typename MsDpfPlusType::range_type, 3>, 2>
            decomp_Hhat_z = {
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one,
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one};

        for (std::size_t party = 0; party < 3; party++) {
          // Zero out the two MSB's of the seeds because block_type consists of 128 bit
          // blocks while lambda is currently 126 bits.
          zero_msbs(x_P[party].bits - lambda, x_P[party]);
          zero_msbs(y_P[party].bits - lambda, y_P[party]);
          zero_msbs(z_P[party].bits - lambda, z_P[party]);

          // Apply the PRG to the seeds and break up the PRG output into the different components.
          prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                           x_P[party],
                                                           prg_output);
          decompose_prg_output<block_type,
                               typename MsDpfPlusType::range_type,
                               prg_output_size>(
              prg_output,
              decomp_G_x[0][party],
              decomp_G_x[1][party],
              decomp_H_x[0][party],
              decomp_Hhat_x[0][party],
              decomp_H_x[1][party],
              decomp_Hhat_x[1][party]);

          prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                           y_P[party],
                                                           prg_output);
          decompose_prg_output<block_type,
                               typename MsDpfPlusType::range_type,
                               prg_output_size>(
              prg_output,
              decomp_G_y[0][party],
              decomp_G_y[1][party],
              decomp_H_y[0][party],
              decomp_Hhat_y[0][party],
              decomp_H_y[1][party],
              decomp_Hhat_y[1][party]);

          prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                           z_P[party],
                                                           prg_output);
          decompose_prg_output<block_type,
                               typename MsDpfPlusType::range_type,
                               prg_output_size>(
              prg_output,
              decomp_G_z[0][party],
              decomp_G_z[1][party],
              decomp_H_z[0][party],
              decomp_Hhat_z[0][party],
              decomp_H_z[1][party],
              decomp_Hhat_z[1][party]);
        }
        // Define variables to hold the secret MS-DPF+ range values.
        std::array<std::vector<typename MsDpfPlusType::range_type>, 2>
            v = {ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                     .vector_lambda,
                 ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                     .vector_lambda};
        std::vector<typename MsDpfPlusType::range_type> v_l =
            ms_dpf_plus_range_type_vector_two_lambda<MsDpfPlusType>()
                .vector_two_lambda;
        std::vector<typename MsDpfPlusType::range_type> vhat_l =
            ms_dpf_plus_range_type_vector_lambda<MsDpfPlusType>()
                .vector_lambda;

        typename MsDpfPlusType::range_type p_range(lambda);
        construct_random_ms_dpf_plus_range_type<block_type>(rng,
                                                            p_range);

        typename MsDpfPlusType::range_type q_range(lambda);
        construct_random_ms_dpf_plus_range_type<block_type>(rng,
                                                            q_range);

        // Set the MS-DPF+ range values for f.
        // If the kth range bit is a 1, we calculate the MS-DPF+
        // secret values according to equations 7 and 8 in BKO21.
        // Otherwise, we set all of the MS-DPF+ shares to the same
        // random value.
        // When the range bit is zero, we need all parties to have the
        // same MS-DPF+ range value. So follow equation 7 for the
        // first party and either use that for all parties if the range
        // bit is zero. Otherwise, we calculate the range values of the
        // other parties according to equation 7 of BKO21.
        const std::size_t stop_at =
            getbitx(range_value, k) == 0 ? 1 : 3;
        for (std::size_t party = 0; party < stop_at; party++) {
          std::size_t Q = party_to_the_left(party);

          v[0][party] = (alpha_i ^ 1) ?
                            (decomp_G_x[0][Q] + (p_range + q_range)) :
                            decomp_G_x[0][Q];
          v[1][party] = alpha_i ?
                            (decomp_G_x[1][Q] + (p_range + q_range)) :
                            decomp_G_x[1][Q];
          v_l[party] = v[0][party] || v[1][party];
        }
        if (getbitx(range_value, k) == 0) {
          for (std::size_t party = 1; party < 3; party++) {
            v[1][party] = v[1][0];
            v[0][party] = v[0][0];
            v_l[party] = v[0][party] || v[1][party];
          }
        }

        ms_dpf_plus_rand_buf_it = generate_ms_dpf_keys<MsDpfPlusType>(
            false,
            k,
            l - 1,
            bko_1_3_security_parameter * 2,
            alpha_l_minus_one,
            v_l,
            pi,
            rng,
            ms_dpf_plus_rand_buf_it,
            key);
        // If we are only on the first level of the binary tree,
        // store the MS-DPF+ range values in the clear.
        if (l - 1 == 0) {
          for (std::size_t party = 0; party < 3; party++) {
            key[party][k].v_L = v[0][party];
            key[party][k].v_R = v[1][party];
          }
        }

        // Calculate correction bits.
        typename MsDpfPlusType::range_type h_L =
            decomp_H_x[0][0] ^ decomp_H_x[0][1] ^ decomp_H_x[0][2]
            ^ decomp_H_y[0][0];
        typename MsDpfPlusType::range_type h_R =
            decomp_H_x[1][0] ^ decomp_H_x[1][1] ^ decomp_H_x[1][2]
            ^ decomp_H_y[1][0];
        typename MsDpfPlusType::range_type hhat_L =
            decomp_Hhat_x[0][0] ^ decomp_Hhat_x[0][1]
            ^ decomp_Hhat_x[0][2] ^ decomp_Hhat_y[0][0];
        typename MsDpfPlusType::range_type hhat_R =
            decomp_Hhat_x[1][0] ^ decomp_Hhat_x[1][1]
            ^ decomp_Hhat_x[1][2] ^ decomp_Hhat_y[1][0];

        // Store correction bits in the keys.
        if (alpha_i == 0) {
          for (std::size_t party = 0; party < 3; party++) {
            key[party][k].r.push_back(h_L);
            key[party][k].s.push_back(z2n_one ^ h_R);
            key[party][k].t.push_back(z2n_one ^ hhat_L);
            key[party][k].u.push_back(hhat_R);
          }
        } else {
          for (std::size_t party = 0; party < 3; party++) {
            key[party][k].r.push_back(z2n_one ^ h_L);
            key[party][k].s.push_back(h_R);
            key[party][k].t.push_back(hhat_L);
            key[party][k].u.push_back(z2n_one ^ hhat_R);
          }
        }

        // Update the control bits for the next level.
        for (std::size_t party = 0; party < 3; party++) {
          if (alpha_i == 0) {
            b_P[party] =
                (c_P[party][0] ? key[party][k].r.back() : z2n_zero)
                ^ decomp_H_x[0][party] ^ decomp_H_y[0][party];
            c_P[party] =
                (c_P[party][0] ? key[party][k].t.back() : z2n_zero)
                ^ decomp_Hhat_x[0][party] ^ decomp_Hhat_y[0][party];
          } else {
            b_P[party] =
                (c_P[party][0] ? key[party][k].s.back() : z2n_zero)
                ^ decomp_H_x[1][party] ^ decomp_H_y[1][party];
            c_P[party] =
                (c_P[party][0] ? key[party][k].u.back() : z2n_zero)
                ^ decomp_Hhat_x[1][party] ^ decomp_Hhat_y[1][party];
          }
        }

        // Use the updated control bits to calculate the MS-DPF+ range
        // value vhat_l. If the kth range bit is zero, set each of the
        // MS-DPF+ range value shares to the same random value, as we
        // did for vhat_l_minus_one.
        if (getbitx(range_value, k) != 0) {
          for (std::size_t party = 0; party < 3; party++) {
            if (b_P[party_to_the_left(party)][0] == z2n_zero[0]) {
              vhat_l[party] = p_range;
            } else {
              vhat_l[party] = q_range;
            }
          }
        } else {
          typename MsDpfPlusType::range_type temp(lambda);
          for (std::size_t i = 0; i < temp.size(); i++) {
            temp[i] =
                static_cast<unsigned char>(*ms_dpf_plus_rand_buf_it++);
          }
          if ((lambda % uchar_bits) != 0) {
            temp[temp.size() - 1] &= get_mask<unsigned char>(
                uchar_bits - (lambda % uchar_bits));
          }
          for (std::size_t party = 0; party < 3; party++) {
            vhat_l[party] = temp;
          }
        }

        // Generate and store the MS-DPF+ keys.
        std::vector<typename MsDpfPlusType::key_type> kappahat_P(
            3,
            typename MsDpfPlusType::key_type(l, lambda));
        ms_dpf_plus_rand_buf_it = generate_ms_dpf_keys<MsDpfPlusType>(
            true,
            k,
            l,
            bko_1_3_security_parameter,
            alpha_l,
            vhat_l,
            pi,
            rng,
            ms_dpf_plus_rand_buf_it,
            key);

        // Update the seeds for the next level, preserving the invariant
        // described in equation 2 from BKO21.
        for (std::size_t party = 0; party < 3; party++) {
          if (b_P[party][0] != b_P[party_to_the_right(party)][0]) {
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_y[alpha_i][party]
                    ^ (b_P[party][0] ? decomp_G_y[alpha_i][party]
                                           ^ v[alpha_i][party] :
                                       z2lambda_zero)
                    ^ vhat_l[party],
                x_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_x[alpha_i][party]
                    ^ (b_P[party][0] ?
                           decomp_G_x[alpha_i][party]
                               ^ decomp_G_z[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                y_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                v[alpha_i][party]
                    ^ (b_P[party][0] ?
                           v[alpha_i][party]
                               ^ decomp_G_y[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                z_P[party]);
          } else {
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_x[alpha_i][party]
                    ^ (b_P[party][0] ?
                           decomp_G_x[alpha_i][party]
                               ^ decomp_G_z[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                x_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_y[alpha_i][party]
                    ^ (b_P[party][0] ? decomp_G_y[alpha_i][party]
                                           ^ v[alpha_i][party] :
                                       z2lambda_zero)
                    ^ vhat_l[party],
                y_P[party]);
            copy_ms_dpf_plus_range_type_to_block(
                decomp_G_z[alpha_i][party]
                    ^ (b_P[party][0] ?
                           decomp_G_z[alpha_i][party]
                               ^ decomp_G_x[alpha_i][party] :
                           z2lambda_zero)
                    ^ vhat_l[party],
                z_P[party]);
          }
        }
      }

      // Compute the final correction word.
      if (getbitx(range_value, k) == 0) {
        // Sample the prg_convert input parameters at random.
        block_type temp_s = rng();
        block_type temp_w = rng();
        zero_msbs(temp_s.bits - lambda, temp_s);
        zero_msbs(temp_w.bits - lambda, temp_w);
        for (std::size_t party = 0; party < 3; party++) {
          key[party][k].convertPRG_index = temp_s;
          key[party][k].w =
              block_to_range_type<block_type, nice_range_type>(
                  range_bits,
                  temp_w);
        }
      } else {
        // Calculate the prg_convert input parameters subject
        // to the constraints described in equations 11, 12,
        // and Appendix A.4 of BKO21.
        block_type convertPRG_index;
        nice_range_type w;
        find_cw(range_bits,
                k,
                lambda,
                pi,
                rng,
                x_P,
                y_P,
                convertPRG_index,
                w);
        for (std::size_t party = 0; party < 3; party++) {
          key[party][k].convertPRG_index = convertPRG_index;
          key[party][k].w = w;
          zero_msbs(key[party][k].convertPRG_index.bits - lambda,
                    key[party][k].convertPRG_index);
        }
      }
    }
  }

  void eval(key_type & key,
            domain_type const & x,
            rand_perm_type & pi,
            std::array<range_type, 2> & result) noexcept {
    // Initialize variables that will be used for XOR-ing
    // with other MS-DPF+ range types.
    typename MsDpfPlusType::range_type z2n_one(1);
    z2n_one[0] = 1;

    typename MsDpfPlusType::range_type z2n_zero(1);
    z2n_zero[0] = 0;

    typename MsDpfPlusType::range_type z2lambda_zero(lambda);

    // Define the variables that will be iteratively updated
    // to contain each leve's seed, control bit, and sibling
    // control bit values.
    block_type x_P;
    block_type y_P;
    block_type z_P;
    block_type prg_output[prg_output_size];
    typename MsDpfPlusType::range_type b_P = z2n_zero;
    typename MsDpfPlusType::range_type bbar_P = z2n_zero;
    typename MsDpfPlusType::range_type c_P = z2n_zero;
    typename MsDpfPlusType::range_type cbar_P = z2n_zero;

    // Define the variables that will hold each level's MS-DPF+
    // eval results.
    std::array<typename MsDpfPlusType::range_type, 2> v =
        ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
            .array_lambda_two_d;
    typename MsDpfPlusType::range_type vhat_P(lambda);
    typename MsDpfPlusType::range_type v_P(lambda * 2);

    // Iterate through each k range bit.
    for (int k = 0; k < range_bits; k++) {
      // Initially set each seed, control bits, and sibling control bits
      // to the initial values stored in the key.
      x_P = key[k].x_0;
      y_P = key[k].y_0;
      z_P = key[k].z_0;
      b_P = key[k].b_0;
      bbar_P = key[k].b_0_party_to_right;
      c_P = key[k].c_0;
      cbar_P = key[k].c_0_party_to_right;

      // Iterate through each domain bit.
      for (int l = 1; l <= domain_bits; l++) {
        // Get the l and l - minus one LSB's of alpha.
        domain_type alpha_l =
            static_cast<domain_type>(get_mask<domain_type>(l) & x);
        domain_type alpha_l_minus_one =
            static_cast<domain_type>(get_mask<domain_type>(l - 1) & x);

        if (l - 1 > 0) {
          MsDpfPlusType MsDpfPlus(l - 1, lambda * 2);
          v_P = MsDpfPlus.eval(key[k].kappa[l - 2],
                               alpha_l_minus_one,
                               pi);
          split_ms_dpf_plus_range_type(v_P, v[0], v[1]);
        } else {
          v[0] = key[k].v_L;
          v[1] = key[k].v_R;
        }

        // Define the variables that will hold the result of plugging
        // in the seeds into the PRG, G.
        std::array<typename MsDpfPlusType::range_type, 2> decomp_G_x =
            ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                .array_lambda_two_d;
        std::array<typename MsDpfPlusType::range_type, 2> decomp_H_x =
            ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                .array_one_two_d;
        std::array<typename MsDpfPlusType::range_type, 2>
            decomp_Hhat_x =
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one_two_d;

        std::array<typename MsDpfPlusType::range_type, 2> decomp_G_y =
            ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                .array_lambda_two_d;
        std::array<typename MsDpfPlusType::range_type, 2> decomp_H_y =
            ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                .array_one_two_d;
        std::array<typename MsDpfPlusType::range_type, 2>
            decomp_Hhat_y =
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one_two_d;

        std::array<typename MsDpfPlusType::range_type, 2> decomp_G_z =
            ms_dpf_plus_range_type_array_lambda<MsDpfPlusType>()
                .array_lambda_two_d;
        std::array<typename MsDpfPlusType::range_type, 2> decomp_H_z =
            ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                .array_one_two_d;
        std::array<typename MsDpfPlusType::range_type, 2>
            decomp_Hhat_z =
                ms_dpf_plus_range_type_array_one<MsDpfPlusType>()
                    .array_one_two_d;

        zero_msbs(x_P.bits - lambda, x_P);
        zero_msbs(y_P.bits - lambda, y_P);
        zero_msbs(z_P.bits - lambda, z_P);

        prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                         x_P,
                                                         prg_output);
        decompose_prg_output<block_type,
                             typename MsDpfPlusType::range_type,
                             prg_output_size>(prg_output,
                                              decomp_G_x[0],
                                              decomp_G_x[1],
                                              decomp_H_x[0],
                                              decomp_Hhat_x[0],
                                              decomp_H_x[1],
                                              decomp_Hhat_x[1]);

        prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                         y_P,
                                                         prg_output);
        decompose_prg_output<block_type,
                             typename MsDpfPlusType::range_type,
                             prg_output_size>(prg_output,
                                              decomp_G_y[0],
                                              decomp_G_y[1],
                                              decomp_H_y[0],
                                              decomp_Hhat_y[0],
                                              decomp_H_y[1],
                                              decomp_Hhat_y[1]);

        prg<block_type, rand_perm_type, prg_output_size>(pi,
                                                         z_P,
                                                         prg_output);
        decompose_prg_output<block_type,
                             typename MsDpfPlusType::range_type,
                             prg_output_size>(prg_output,
                                              decomp_G_z[0],
                                              decomp_G_z[1],
                                              decomp_H_z[0],
                                              decomp_Hhat_z[0],
                                              decomp_H_z[1],
                                              decomp_Hhat_z[1]);

        // Update the values of the control bits and sibling control bits.
        bool const x_i = getbitx(x, l - 1);
        if (!x_i) {
          b_P = (key[k].r[l - 1][0] ? c_P : z2n_zero) ^ decomp_H_x[0]
                ^ decomp_H_y[0];
          bbar_P = (key[k].r[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_H_y[0] ^ decomp_H_z[0];
          c_P = (key[k].t[l - 1][0] ? c_P : z2n_zero) ^ decomp_Hhat_x[0]
                ^ decomp_Hhat_y[0];
          cbar_P = (key[k].t[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_Hhat_y[0] ^ decomp_Hhat_z[0];
        } else {
          b_P = (key[k].s[l - 1][0] ? c_P : z2n_zero) ^ decomp_H_x[1]
                ^ decomp_H_y[1];
          bbar_P = (key[k].s[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_H_y[1] ^ decomp_H_z[1];
          c_P = (key[k].u[l - 1][0] ? c_P : z2n_zero) ^ decomp_Hhat_x[1]
                ^ decomp_Hhat_y[1];
          cbar_P = (key[k].u[l - 1][0] ? cbar_P : z2n_zero)
                   ^ decomp_Hhat_y[1] ^ decomp_Hhat_z[1];
        }

        MsDpfPlusType MsDpfPlusHat(l, lambda);
        vhat_P = MsDpfPlusHat.eval(key[k].kappahat[l - 1], alpha_l, pi);

        // Update the values of the seeds for the next level.
        if (b_P[0] != bbar_P[0]) {
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_y[x_i]
                  ^ (b_P[0] ? decomp_G_y[x_i] ^ v[x_i] : z2lambda_zero)
                  ^ vhat_P,
              x_P);
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_x[x_i]
                  ^ (b_P[0] ? decomp_G_x[x_i] ^ decomp_G_z[x_i] :
                              z2lambda_zero)
                  ^ vhat_P,
              y_P);
          copy_ms_dpf_plus_range_type_to_block(
              v[x_i]
                  ^ (b_P[0] ? v[x_i] ^ decomp_G_y[x_i] : z2lambda_zero)
                  ^ vhat_P,
              z_P);
        } else {
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_x[x_i]
                  ^ (b_P[0] ? decomp_G_x[x_i] ^ decomp_G_z[x_i] :
                              z2lambda_zero)
                  ^ vhat_P,
              x_P);
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_y[x_i]
                  ^ (b_P[0] ? decomp_G_y[x_i] ^ v[x_i] : z2lambda_zero)
                  ^ vhat_P,
              y_P);
          copy_ms_dpf_plus_range_type_to_block(
              decomp_G_z[x_i]
                  ^ (b_P[0] ? decomp_G_z[x_i] ^ decomp_G_x[x_i] :
                              z2lambda_zero)
                  ^ vhat_P,
              z_P);
        }
      }

      // Calculate the CNF-shared eval results based on the methods
      // described in appendix A.4 of BKO21.
      zero_msbs(x_P.bits - lambda, x_P);
      zero_msbs(y_P.bits - lambda, y_P);
      zero_msbs(z_P.bits - lambda, z_P);
      const nice_range_type m = static_cast<nice_range_type>(1)
                                << (range_bits - k);
      const nice_range_type ghat_x =
          prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
              range_bits,
              range_bits - k,
              pi,
              x_P,
              key[k].convertPRG_index);
      const nice_range_type ghat_y =
          prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
              range_bits,
              range_bits - k,
              pi,
              y_P,
              key[k].convertPRG_index);
      const nice_range_type ghat_z =
          prg_convert_for_power_of_two_groups<block_type,
                                              rand_perm_type,
                                              nice_range_type>(
              range_bits,
              range_bits - k,
              pi,
              z_P,
              key[k].convertPRG_index);
      uint64_t rhs =
          (((key[k].w * ghat_x % m) - (key[k].w * ghat_y % m)) % m);
      result[0] += rhs * (static_cast<uint64_t>(1) << (k));

      rhs = (((key[k].w * ghat_z % m) - (key[k].w * ghat_y % m)) % m);
      result[1] += rhs * (static_cast<uint64_t>(1) << (k));
    }
  }
};

} // namespace pfss

#endif // PFSS_1_3_CNF_FSS_H
