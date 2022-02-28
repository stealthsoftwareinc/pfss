//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_BKO_MS_DPF_PLUS_H
#define PFSS_BKO_MS_DPF_PLUS_H

//
// This file implements the MS-DPF+ protocol from <https://eprint.iacr.org/2021/163>. For
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

#include <pfss/bko_two_party_ms_dpf_plus.h>
#include <pfss/common.h>

namespace pfss {
template<class MsDpfType>
class bko_ms_dpf_plus final {
public:
  static constexpr int domain_bits = MsDpfType::domain_bits;
  static constexpr int range_bits = MsDpfType::range_bits;
  using domain_type = typename MsDpfType::domain_type;
  using range_type = typename MsDpfType::range_type;
  using block_type = typename MsDpfType::block_type;
  using rand_perm_type = typename MsDpfType::rand_perm_type;
  using rng_type = typename MsDpfType::rng_type;
  static constexpr bool range_pack = MsDpfType::range_pack;
  static constexpr bool prg_convert = MsDpfType::prg_convert;

  PFSS_SST_STATIC_ASSERT(domain_bits > 0);
  PFSS_SST_STATIC_ASSERT(range_bits > 0);

  bko_ms_dpf_plus() noexcept = default;
  bko_ms_dpf_plus(int const db, int const rb) noexcept {
    assert(db == domain_bits);
    assert(rb == range_bits);
  }

  ~bko_ms_dpf_plus() noexcept = default;
  bko_ms_dpf_plus(bko_ms_dpf_plus const &) noexcept = default;
  bko_ms_dpf_plus(bko_ms_dpf_plus &&) noexcept = default;
  bko_ms_dpf_plus &
  operator=(bko_ms_dpf_plus const &) noexcept = default;
  bko_ms_dpf_plus & operator=(bko_ms_dpf_plus &&) noexcept = default;

  static constexpr std::size_t key_blob_size() noexcept {
    return key_type::size();
  }

  static constexpr std::size_t rand_buf_size() noexcept {
    return 3 * (block_type().size() * 2);
  }

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {4};
  }

  struct key_type final {

    static constexpr int domain_bits = MsDpfType::domain_bits;
    static constexpr int range_bits = MsDpfType::range_bits;
    uint8_t party;
    std::array<typename MsDpfType::key_type, 3>
        two_party_ms_dpf_plus_keys;

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

          // The three other keys.
          3*MsDpfType::key_blob_size()

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
      out = two_party_ms_dpf_plus_keys[0].serialize(out);
      out = two_party_ms_dpf_plus_keys[1].serialize(out);
      out = two_party_ms_dpf_plus_keys[2].serialize(out);
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
        std::cout << static_cast<int>(pt) << std::endl;
        throw std::invalid_argument("key blob is corrupt");
      }
      party = static_cast<uint8_t>(pt);

      in = two_party_ms_dpf_plus_keys[0].parse(in);
      in = two_party_ms_dpf_plus_keys[1].parse(in);
      in = two_party_ms_dpf_plus_keys[2].parse(in);

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

  static void gen(std::vector<key_type> & key,
                  domain_type const & domain_slot,
                  std::vector<range_type> const & range_value_shares,
                  std::vector<range_type> const & rand_offsets,
                  rand_perm_type & pi,
                  rng_type & rng) noexcept {
    std::vector<range_type> v0(2);
    std::vector<range_type> v1(2);
    std::vector<range_type> v2(2);
    v0[0] = range_value_shares[0] ^ rand_offsets[1] ^ rand_offsets[2];
    v1[0] = rand_offsets[0] ^ range_value_shares[1] ^ rand_offsets[2];
    v2[0] = rand_offsets[0] ^ rand_offsets[1] ^ range_value_shares[2];
    v0[1] = rand_offsets[0];
    v1[1] = rand_offsets[1];
    v2[1] = rand_offsets[2];

    std::vector<typename MsDpfType::key_type>
        two_party_ms_dpf_plus_keys0(2);
    std::vector<typename MsDpfType::key_type>
        two_party_ms_dpf_plus_keys1(2);
    std::vector<typename MsDpfType::key_type>
        two_party_ms_dpf_plus_keys2(2);

    MsDpfType::gen(two_party_ms_dpf_plus_keys0,
                   domain_slot,
                   v0,
                   pi,
                   rng);
    MsDpfType::gen(two_party_ms_dpf_plus_keys1,
                   domain_slot,
                   v1,
                   pi,
                   rng);
    MsDpfType::gen(two_party_ms_dpf_plus_keys2,
                   domain_slot,
                   v2,
                   pi,
                   rng);
    key[0].party = 0;
    key[0].two_party_ms_dpf_plus_keys[0] =
        two_party_ms_dpf_plus_keys0[0];
    key[0].two_party_ms_dpf_plus_keys[1] =
        two_party_ms_dpf_plus_keys1[1];
    key[0].two_party_ms_dpf_plus_keys[2] =
        two_party_ms_dpf_plus_keys2[1];

    key[1].party = 1;
    key[1].two_party_ms_dpf_plus_keys[0] =
        two_party_ms_dpf_plus_keys0[1];
    key[1].two_party_ms_dpf_plus_keys[1] =
        two_party_ms_dpf_plus_keys1[0];
    key[1].two_party_ms_dpf_plus_keys[2] =
        two_party_ms_dpf_plus_keys2[1];

    key[2].party = 2;
    key[2].two_party_ms_dpf_plus_keys[0] =
        two_party_ms_dpf_plus_keys0[1];
    key[2].two_party_ms_dpf_plus_keys[1] =
        two_party_ms_dpf_plus_keys1[1];
    key[2].two_party_ms_dpf_plus_keys[2] =
        two_party_ms_dpf_plus_keys2[0];
  }

  static range_type eval(key_type const & key,
                         domain_type const & x,
                         rand_perm_type & pi) noexcept {
    return MsDpfType::eval(key.two_party_ms_dpf_plus_keys[0], x, pi)
           ^ MsDpfType::eval(key.two_party_ms_dpf_plus_keys[1], x, pi)
           ^ MsDpfType::eval(key.two_party_ms_dpf_plus_keys[2], x, pi);
  }
};

//
// Runtime domain_bits and range_bits.
//

template<class MsDpfType>
class bko_ms_dpf_plus_v final {
public:
  int const domain_bits;
  int const range_bits;
  using domain_type = typename MsDpfType::domain_type;
  using range_type = typename MsDpfType::range_type;
  using block_type = typename MsDpfType::block_type;
  using rand_perm_type = typename MsDpfType::rand_perm_type;
  using rng_type = typename MsDpfType::rng_type;
  static constexpr bool range_pack = MsDpfType::range_pack;
  static constexpr bool prg_convert = MsDpfType::prg_convert;

  bko_ms_dpf_plus_v(int const domain_bits,
                    int const range_bits) noexcept
      : domain_bits(domain_bits),
        range_bits(range_bits) {
    assert(domain_bits > 0);
    assert(range_bits > 0);
  }

  ~bko_ms_dpf_plus_v() noexcept = default;
  bko_ms_dpf_plus_v(bko_ms_dpf_plus_v const &) noexcept = default;
  bko_ms_dpf_plus_v(bko_ms_dpf_plus_v &&) noexcept = default;
  bko_ms_dpf_plus_v &
  operator=(bko_ms_dpf_plus_v const &) noexcept = default;
  bko_ms_dpf_plus_v &
  operator=(bko_ms_dpf_plus_v &&) noexcept = default;

  constexpr std::size_t key_blob_size() const noexcept {
    return key_type(domain_bits, range_bits).size();
  }

  constexpr std::size_t rand_buf_size() noexcept {
    return 3 * (block_type().size() * 2);
  }

  static constexpr std::array<unsigned char, 1> key_header() noexcept {
    return {4};
  }

  struct key_type final {
    int const domain_bits;
    int const range_bits;
    uint8_t party;
    std::array<typename MsDpfType::key_type, 3>
        two_party_ms_dpf_plus_keys;

    key_type(int const domain_bits, int const range_bits)
        : domain_bits(domain_bits),
          range_bits(range_bits),
          two_party_ms_dpf_plus_keys{
              typename MsDpfType::key_type(domain_bits, range_bits),
              typename MsDpfType::key_type(domain_bits, range_bits),
              typename MsDpfType::key_type(domain_bits, range_bits)} {
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

          // The three other keys.
          3*MsDpfType(domain_bits, range_bits).key_blob_size()

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
      out = two_party_ms_dpf_plus_keys[0].serialize(out);
      out = two_party_ms_dpf_plus_keys[1].serialize(out);
      out = two_party_ms_dpf_plus_keys[2].serialize(out);
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
      party = static_cast<bool>(pt);

      in = two_party_ms_dpf_plus_keys[0].parse(in);
      in = two_party_ms_dpf_plus_keys[1].parse(in);
      in = two_party_ms_dpf_plus_keys[2].parse(in);

      return in;
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
           domain_type const & domain_slot,
           std::vector<range_type> const & range_value_shares,
           std::vector<range_type> const & rand_offsets,
           rand_perm_type & pi,
           rng_type & rng) noexcept {

    std::vector<range_type> v0(2, range_type(range_bits));
    std::vector<range_type> v1(2, range_type(range_bits));
    std::vector<range_type> v2(2, range_type(range_bits));
    v0[0] = range_value_shares[0] ^ rand_offsets[1] ^ rand_offsets[2];
    v1[0] = rand_offsets[0] ^ range_value_shares[1] ^ rand_offsets[2];
    v2[0] = rand_offsets[0] ^ rand_offsets[1] ^ range_value_shares[2];
    v0[1] = rand_offsets[0];
    v1[1] = rand_offsets[1];
    v2[1] = rand_offsets[2];

    std::vector<typename MsDpfType::key_type>
        two_party_ms_dpf_plus_keys0(
            2,
            typename MsDpfType::key_type(domain_bits, range_bits));
    std::vector<typename MsDpfType::key_type>
        two_party_ms_dpf_plus_keys1(
            2,
            typename MsDpfType::key_type(domain_bits, range_bits));
    std::vector<typename MsDpfType::key_type>
        two_party_ms_dpf_plus_keys2(
            2,
            typename MsDpfType::key_type(domain_bits, range_bits));

    MsDpfType two_party_ms_dpf_plus(domain_bits, range_bits);
    two_party_ms_dpf_plus.gen(two_party_ms_dpf_plus_keys0,
                              domain_slot,
                              v0,
                              pi,
                              rng);
    two_party_ms_dpf_plus.gen(two_party_ms_dpf_plus_keys1,
                              domain_slot,
                              v1,
                              pi,
                              rng);
    two_party_ms_dpf_plus.gen(two_party_ms_dpf_plus_keys2,
                              domain_slot,
                              v2,
                              pi,
                              rng);
    key[0].party = 0;
    key[0].two_party_ms_dpf_plus_keys[0] =
        two_party_ms_dpf_plus_keys0[0];
    key[0].two_party_ms_dpf_plus_keys[1] =
        two_party_ms_dpf_plus_keys1[1];
    key[0].two_party_ms_dpf_plus_keys[2] =
        two_party_ms_dpf_plus_keys2[1];

    key[1].party = 1;
    key[1].two_party_ms_dpf_plus_keys[0] =
        two_party_ms_dpf_plus_keys0[1];
    key[1].two_party_ms_dpf_plus_keys[1] =
        two_party_ms_dpf_plus_keys1[0];
    key[1].two_party_ms_dpf_plus_keys[2] =
        two_party_ms_dpf_plus_keys2[1];

    key[2].party = 2;
    key[2].two_party_ms_dpf_plus_keys[0] =
        two_party_ms_dpf_plus_keys0[1];
    key[2].two_party_ms_dpf_plus_keys[1] =
        two_party_ms_dpf_plus_keys1[1];
    key[2].two_party_ms_dpf_plus_keys[2] =
        two_party_ms_dpf_plus_keys2[0];
  }

  range_type eval(key_type const & key,
                  domain_type const & x,
                  rand_perm_type & pi) noexcept {
    MsDpfType two_party_ms_dpf_plus(domain_bits, range_bits);
    return two_party_ms_dpf_plus.eval(key.two_party_ms_dpf_plus_keys[0],
                                      x,
                                      pi)
           ^ two_party_ms_dpf_plus.eval(
               key.two_party_ms_dpf_plus_keys[1],
               x,
               pi)
           ^ two_party_ms_dpf_plus.eval(
               key.two_party_ms_dpf_plus_keys[2],
               x,
               pi);
  }
};

} // namespace pfss

#endif // PFSS_BKO_MS_DPF_PLUS_H
