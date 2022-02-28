/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <iostream>
#include <pfss.h>
#include <pfss/common.h>
#include <pfss/config.h>
#include <string>

using namespace pfss;

template<class Scheme>
class ms_dpf_plus_v_test_helper {
public:
  int const domain_bits;
  int const range_bits;
  int const NumParties = 3;
  using DomainType = typename Scheme::domain_type;
  using RangeType = typename Scheme::range_type;

  std::vector<typename Scheme::key_type> keys;
  std::vector<typename Scheme::key_type> deserialized_keys;
  // initialize containers for range values. Each container is the
  // size of the full domain
  typename std::vector<typename Scheme::range_type> ys;

  unsigned char aeskey[16] = {0};
  typename Scheme::rand_perm_type pi{aeskey};

  ms_dpf_plus_v_test_helper(int const & DomainBits, int const & RangeBits)
      : domain_bits(DomainBits),
        range_bits(RangeBits) {
    ys =
        std::vector<typename Scheme::range_type>(NumParties,
                                                 RangeType(range_bits));
    keys = std::vector<typename Scheme::key_type>(
        NumParties,
        typename Scheme::key_type(domain_bits, range_bits));
    deserialized_keys = std::vector<typename Scheme::key_type>(
        NumParties,
        typename Scheme::key_type(domain_bits, range_bits));
  }

  void do_up_to_gen(DomainType domain_slot,
                    std::vector<RangeType> range_value_shares,
                    typename Scheme::rng_type & rng) {
    RangeType offset0(range_bits);
    RangeType offset1(range_bits);
    RangeType offset2(range_bits);
    std::vector<RangeType> rand_offsets = {offset0, offset1, offset2};
    for (auto & offset : rand_offsets) {
      for (std::size_t i = 0; i < offset.size(); i++) {
        offset[i] = static_cast<unsigned char>(std::rand());
      }
      if (range_bits % uchar_bits != 0) {
        offset[offset.size() - 1] &= get_mask<unsigned char>(
            uchar_bits - (range_bits % uchar_bits));
      }
    }

    Scheme Fss(domain_bits, range_bits);
    Fss.gen(keys,
            domain_slot,
            range_value_shares,
            rand_offsets,
            pi,
            rng);
  }
};

template<class Scheme>
bool ms_dpf_plus_test_correctness_with_serialization(
    typename Scheme::rng_type & r,
    typename Scheme::domain_type const & domain_slot,
    int const domain_bits,
    int const range_bits) {
  int exit_status = TEST_EXIT_PASS;
  ms_dpf_plus_v_test_helper<Scheme> t(domain_bits, range_bits);
  // std::cout << "starting gen\n";

  typename Scheme::range_type z2n_zero(range_bits);

  std::vector<typename Scheme::range_type> range_value_shares = {
      z2n_zero,
      z2n_zero,
      z2n_zero};
  for (auto & share : range_value_shares) {
    for (std::size_t i = 0; i < share.size(); i++) {
      share[i] = static_cast<unsigned char>(std::rand());
    }
    if (range_bits % uchar_bits != 0) {
      share[share.size() - 1] &= get_mask<unsigned char>(
          uchar_bits - (range_bits % uchar_bits));
    }
  }

  t.do_up_to_gen(domain_slot, range_value_shares, r);

  Scheme Fss(domain_bits, range_bits);
  for (int i = 0; i < t.NumParties; i++) {
    std::vector<unsigned char> blob(Fss.key_blob_size());
    // std::cout << blob.size() << std::endl;
    t.keys[i].serialize(blob.begin());
    t.deserialized_keys[i].parse(blob.cbegin());
  }

  for (typename Scheme::domain_type x = 0; x < 2000; ++x) {
    // std::cout << x << std::endl;
    for (int i = 0; i < t.NumParties; i++) {
      t.ys[i] = Fss.eval(t.deserialized_keys[i], x, t.pi);
    }

    if (x == domain_slot) {
      for (int i = 0; i < t.NumParties; i++) {
        if (t.ys[i].size() != range_value_shares[i].size()) {
          std::cout << "incorrect eval result size" << std::endl;
          exit_status = TEST_EXIT_FAIL;
        }
        for (std::size_t j = 0; j < t.ys[i].size(); j++) {
          if (t.ys[i][j] != range_value_shares[i][j]) {
            std::cout << "plus property failure" << std::endl;
            exit_status = TEST_EXIT_FAIL;
          }
        }
      }
    } else {
      bool shares_match =
          std::find_if(
              t.ys.begin() + 1,
              t.ys.end(),
              std::bind1st(
                  std::not_equal_to<typename Scheme::range_type>(),
                  t.ys.front()))
          == t.ys.end();
      if (!shares_match) {
        std::cout << "matching share property failure" << std::endl;
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }

  return exit_status == TEST_EXIT_PASS;
}
