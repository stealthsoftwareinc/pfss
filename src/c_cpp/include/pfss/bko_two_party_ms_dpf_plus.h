//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BKO_TWO_PARTY_MS_DPF_PLUS_H
#define PFSS_BKO_TWO_PARTY_MS_DPF_PLUS_H

//
// This file implements the 2-party MS-DPF+ protocol from <https://eprint.iacr.org/2021/163>. For
// reference, here is a complete citation:
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

#include <pfss/bgi1.h>
#include <pfss/common.h>

namespace pfss {

template<class TwoPartyFssType>
class bko_two_party_ms_dpf_plus final {
public:
  static constexpr int domain_bits = TwoPartyFssType::domain_bits;
  static constexpr int range_bits = TwoPartyFssType::range_bits;
  using domain_type = typename TwoPartyFssType::domain_type;
  using range_type = typename TwoPartyFssType::range_type;
  using block_type = typename TwoPartyFssType::block_type;
  using rand_perm_type = typename TwoPartyFssType::rand_perm_type;
  using rng_type = typename TwoPartyFssType::rng_type;
  static constexpr bool range_pack = TwoPartyFssType::range_pack;
  static constexpr bool prg_convert = TwoPartyFssType::prg_convert;

  PFSS_SST_STATIC_ASSERT(domain_bits > 0);
  PFSS_SST_STATIC_ASSERT(range_bits > 0);

public:
  bko_two_party_ms_dpf_plus() noexcept = default;
  bko_two_party_ms_dpf_plus(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bko_two_party_ms_dpf_plus() noexcept = default;
  bko_two_party_ms_dpf_plus(
      bko_two_party_ms_dpf_plus const &) noexcept = default;
  bko_two_party_ms_dpf_plus(bko_two_party_ms_dpf_plus &&) noexcept =
      default;
  bko_two_party_ms_dpf_plus &
  operator=(bko_two_party_ms_dpf_plus const &) noexcept = default;
  bko_two_party_ms_dpf_plus &
  operator=(bko_two_party_ms_dpf_plus &&) noexcept = default;

  static constexpr std::size_t key_blob_size() noexcept {
    return key_type::size();
  }

  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {3};
  }

  struct key_type final {

    static constexpr int domain_bits = TwoPartyFssType::domain_bits;
    static constexpr int range_bits = TwoPartyFssType::range_bits;
    uint8_t party;
    range_type z;
    typename TwoPartyFssType::key_type two_party_fss_key;

    static constexpr std::size_t size() noexcept {
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

          // The range offset z.
          bits_to_bytes(range_bits) +

          TwoPartyFssType::key_blob_size()

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
      out = z.serialize(out);
      out = two_party_fss_key.serialize(out);
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
      if (pt > 1) {
        throw std::invalid_argument("key blob is corrupt");
      }
      party = static_cast<bool>(pt);

      in = z.parse(in);

      in = two_party_fss_key.parse(in);
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

  template<class KeyType>
  static void gen(std::vector<KeyType> & key,
                  domain_type const & alpha,
                  std::vector<range_type> const & range_value_shares,
                  rand_perm_type & pi,
                  rng_type & rng) noexcept {
    range_type v = range_value_shares[0] ^ range_value_shares[1];
    typename TwoPartyFssType::key_type two_party_fss_keys[2];

    TwoPartyFssType::gen(two_party_fss_keys, alpha, v, pi, rng);

    range_type z =
        range_value_shares[0]
        ^ TwoPartyFssType::eval(two_party_fss_keys[0], alpha, pi);

    key[0].party = 0;
    key[1].party = 1;

    key[0].z = z;
    key[0].two_party_fss_key = two_party_fss_keys[0];

    key[1].z = z;
    key[1].two_party_fss_key = two_party_fss_keys[1];
  }

  static range_type eval(key_type const & key,
                         domain_type const & x,
                         rand_perm_type & pi) noexcept {
    return key.z ^ TwoPartyFssType::eval(key.two_party_fss_key, x, pi);
  }
};

//
// Runtime domain_bits and range_bits.
//

template<class TwoPartyFssType>
class bko_two_party_ms_dpf_plus_v final {
public:
  int const domain_bits;
  int const range_bits;
  using domain_type = typename TwoPartyFssType::domain_type;
  using range_type = typename TwoPartyFssType::range_type;
  using block_type = typename TwoPartyFssType::block_type;
  using rand_perm_type = typename TwoPartyFssType::rand_perm_type;
  using rng_type = typename TwoPartyFssType::rng_type;
  static constexpr bool range_pack = TwoPartyFssType::range_pack;
  static constexpr bool prg_convert = TwoPartyFssType::prg_convert;

public:
  bko_two_party_ms_dpf_plus_v(int const domain_bits,
                              int const range_bits) noexcept
      : domain_bits(domain_bits),
        range_bits(range_bits) {
    assert(domain_bits > 0);
    assert(range_bits > 0);
  }

  ~bko_two_party_ms_dpf_plus_v() noexcept = default;
  bko_two_party_ms_dpf_plus_v(
      bko_two_party_ms_dpf_plus_v const &) noexcept = default;
  bko_two_party_ms_dpf_plus_v(bko_two_party_ms_dpf_plus_v &&) noexcept =
      default;
  bko_two_party_ms_dpf_plus_v &
  operator=(bko_two_party_ms_dpf_plus_v const &) noexcept = default;
  bko_two_party_ms_dpf_plus_v &
  operator=(bko_two_party_ms_dpf_plus_v &&) noexcept = default;

  constexpr std::size_t key_blob_size() const noexcept {
    return key_type(domain_bits, range_bits).size();
  }

  static constexpr std::size_t rand_buf_size() noexcept {
    return block_type().size() * 2;
  }

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {3};
  }

  struct key_type final {

    int domain_bits;
    int range_bits;
    uint8_t party;
    range_type z;
    typename TwoPartyFssType::key_type two_party_fss_key;

    key_type(int const domain_bits, int const range_bits)
        : domain_bits(domain_bits),
          range_bits(range_bits),
          z(range_bits),
          two_party_fss_key(domain_bits, range_bits) {
    }

    constexpr std::size_t size() noexcept {
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

          // The range offset z.
          T(bits_to_bytes(range_bits)) +

          // The three other keys.
          T(TwoPartyFssType(domain_bits, range_bits).key_blob_size())

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
      out = z.serialize(out);
      out = two_party_fss_key.serialize(out);
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
        throw std::invalid_argument("db key blob is corrupt");
      }

      unsigned char const rb = *in++;
      if (unsigned_ne(rb, range_bits)) {
        throw std::invalid_argument("rb key blob is corrupt");
      }

      unsigned char const pt = *in++;
      if (pt > 1) {
        std::cout << static_cast<int>(pt) << std::endl;
        throw std::invalid_argument("pt key blob is corrupt");
      }
      party = static_cast<uint8_t>(pt);

      in = z.parse(in);

      in = two_party_fss_key.parse(in);

      return in;
    }

    key_type & operator=(key_type const & other) noexcept {
      this->domain_bits = other.domain_bits;
      this->range_bits = other.range_bits;
      this->party = other.party;
      this->z = other.z;
      this->two_party_fss_key = other.two_party_fss_key;
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

  void gen(std::vector<key_type> & key,
           domain_type const & alpha,
           std::vector<range_type> const & range_value_shares,
           rand_perm_type & pi,
           rng_type & rng) noexcept {
    range_type v = range_value_shares[0] ^ range_value_shares[1];
    typename TwoPartyFssType::key_type two_party_fss_keys[2] = {
        typename TwoPartyFssType::key_type(domain_bits, range_bits),
        typename TwoPartyFssType::key_type(domain_bits, range_bits)};

    TwoPartyFssType fss(domain_bits, range_bits);
    fss.gen(two_party_fss_keys, alpha, v, pi, rng);

    range_type z = range_value_shares[0]
                   ^ fss.eval(two_party_fss_keys[0], alpha, pi);

    key[0].party = 0;
    key[0].z = z;
    key[0].two_party_fss_key = two_party_fss_keys[0];

    key[1].party = 1;
    key[1].z = z;
    key[1].two_party_fss_key = two_party_fss_keys[1];
  }

  range_type eval(key_type const & key,
                  domain_type const & x,
                  rand_perm_type & pi) noexcept {
    TwoPartyFssType fss(domain_bits, range_bits);
    return key.z ^ fss.eval(key.two_party_fss_key, x, pi);
  }
};

} // namespace pfss

#endif // PFSS_BKO_TWO_PARTY_MS_DPF_PLUS_H
