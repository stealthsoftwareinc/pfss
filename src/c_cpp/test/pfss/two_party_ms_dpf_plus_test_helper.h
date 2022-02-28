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
class two_party_ms_dpf_plus_v_test_helper {
public:
  int const domain_bits;
  int const range_bits;
  int const NumParties = 2;
  using DomainType = typename Scheme::domain_type;
  using RangeType = typename Scheme::range_type;
  static constexpr DomainType domain_mask =
      get_mask<DomainType>(Scheme::domain_bits);

  std::vector<typename Scheme::key_type> keys;
  std::vector<typename Scheme::key_type> deserialized_keys;
  // initialize containers for range values. Each container is the
  // size of the full domain
  typename std::vector<RangeType> ys;

  unsigned char aeskey[16] = {0};
  typename Scheme::rand_perm_type pi{aeskey};

  two_party_ms_dpf_plus_v_test_helper(int const & DomainBits,
                                    int const & RangeBits)
      : domain_bits(DomainBits),
        range_bits(RangeBits) {
    ys = std::vector<RangeType>(NumParties, RangeType(range_bits));
    keys = std::vector<typename Scheme::key_type>(
        NumParties,
        typename Scheme::key_type(domain_bits, range_bits));
    deserialized_keys = std::vector<typename Scheme::key_type>(
        NumParties,
        typename Scheme::key_type(domain_bits, range_bits));
  }

  void do_up_to_gen(DomainType const & domain_slot,
                    std::vector<RangeType> const & range_value_shares,
                    typename Scheme::rng_type & rng) {
    Scheme Fss(domain_bits, range_bits);
    Fss.gen(keys, domain_slot, range_value_shares, pi, rng);
  }
};

template<class Scheme>
bool two_party_ms_dpf_plus_test_correctness_with_serialization(
    typename Scheme::rng_type & r,
    typename Scheme::domain_type const & domain_slot,
    int const domain_bits,
    int const range_bits) {
  using test_helper_type = two_party_ms_dpf_plus_v_test_helper<Scheme>;
  int exit_status = TEST_EXIT_PASS;

  typename Scheme::range_type z2n_one(range_bits);
  z2n_one[0] = 1;

  typename Scheme::range_type z2n_zero(range_bits);

  std::vector<typename test_helper_type::RangeType> range_value_shares =
      {z2n_zero, z2n_one};

  test_helper_type t(domain_bits, range_bits);
  Scheme Fss(domain_bits, range_bits);

  t.do_up_to_gen(domain_slot, range_value_shares, r);
  for (int i = 0; i < t.NumParties; i++) {
    std::vector<unsigned char> blob(Fss.key_blob_size());
    t.keys[i].serialize(blob.begin());
    t.deserialized_keys[i].parse(blob.cbegin());
  }

  std::vector<typename Scheme::range_type> ys(
      t.NumParties,
      typename Scheme::range_type(range_bits));
  for (typename Scheme::domain_type x = 0; x < 2000; ++x) {
    for (int i = 0; i < t.NumParties; i++) {
      ys[i] = Fss.eval(t.deserialized_keys[i], x, t.pi);
    }

    if (x == domain_slot) {
      typename Scheme::range_type y(range_bits);
      for (int i = 0; i < t.NumParties; i++) {
        if (ys[i] != range_value_shares[i]) {
          std::cout << "failure to satisfy plus property" << std::endl;
          exit_status = TEST_EXIT_FAIL;
        }
      }
    } else {
      bool shares_match =
          std::find_if(
              ys.begin() + 1,
              ys.end(),
              std::bind1st(
                  std::not_equal_to<typename Scheme::range_type>(),
                  ys.front()))
          == ys.end();
      if (!shares_match) {
        std::cout << "failure to satisfy matching share property"
                  << std::endl;
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }

  return exit_status == TEST_EXIT_PASS;
}
