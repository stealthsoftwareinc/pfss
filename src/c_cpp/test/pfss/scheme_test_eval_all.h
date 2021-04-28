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
class test_helper {
public:
  using DomainType = typename Scheme::domain_type;
  using RangeType = typename Scheme::range_type;
  static constexpr DomainType domain_mask =
      get_mask<DomainType>(Scheme::domain_bits);
  static constexpr typename Scheme::nice_range_type range_mask =
      get_mask<RangeType>(Scheme::range_bits);

  typename Scheme::key_type keys[2];
  typename std::vector<typename Scheme::nice_range_type> ys0 =
      decltype(ys0)(domain_mask + 1, 0);
  typename std::vector<typename Scheme::nice_range_type> ys1 =
      decltype(ys1)(domain_mask + 1, 0);
  typename Scheme::key_type deserialized_key0;
  typename Scheme::key_type deserialized_key1;

  unsigned char aeskey[16] = {0};
  typename Scheme::rand_perm_type pi{aeskey};

  void do_up_to_gen(DomainType domain_slot,
                    RangeType range_value,
                    typename Scheme::rng_type & rng) {
    Scheme::gen(keys, domain_slot, range_value, pi, rng);
  }

  void eval_all_deserialized() {
    Scheme::eval_all(deserialized_key0, ys0, pi);
    Scheme::eval_all(deserialized_key1, ys1, pi);
  }
};

template<class Scheme, int NumParties>
class p_party_test_helper {
public:
  using DomainType = typename Scheme::domain_type;
  using RangeType = typename Scheme::range_type;
  static constexpr DomainType domain_mask =
      get_mask<DomainType>(Scheme::domain_bits);
  static constexpr typename Scheme::nice_range_type range_mask =
      get_mask<RangeType>(Scheme::range_bits);

  std::vector<typename Scheme::key_type> keys;
  typename std::vector<std::vector<typename Scheme::nice_range_type>>
      ys;
  std::vector<typename Scheme::key_type> deserialized_keys;

  unsigned char aeskey[16] = {0};
  typename Scheme::rand_perm_type pi{aeskey};

  p_party_test_helper() {
    ys = std::vector<std::vector<typename Scheme::nice_range_type>>(
        NumParties,
        typename decltype(ys)::value_type(domain_mask + 1, 0));
    keys.resize(NumParties);
    deserialized_keys.resize(NumParties);
  }

  void do_up_to_gen(DomainType domain_slot,
                    RangeType range_value,
                    typename Scheme::rng_type & rng) {
    std::vector<unsigned char> rand_buf(Scheme::ep_op_rand_buf_size());
    for (auto & byte : rand_buf) {
      byte = rand();
    }
    Scheme::gen(keys,
                domain_slot,
                range_value,
                pi,
                rng,
                rand_buf.begin());
  }

  void eval_all_deserialized() {
    for (int i = 0; i < NumParties; i++) {
      Scheme::eval_all(deserialized_keys[i], ys[i], pi);
    }
  }
};

template<class Scheme>
bool scheme_test_correctness(typename Scheme::rng_type & r,
                             typename Scheme::domain_type domain_slot) {
  int exit_status = TEST_EXIT_PASS;
  test_helper<Scheme> t;
  t.do_up_to_gen(domain_slot, 256, r);

  t.deserialized_key0 = t.keys[0];
  t.deserialized_key1 = t.keys[1];

  t.eval_all_deserialized();

  for (typename Scheme::domain_type x = 0; x < 2048; ++x) {
    auto const y = (t.ys0[x] + t.ys1[x]) & t.range_mask;
    std::cout << std::to_string(x) << ": " << std::to_string(t.ys0[x])
              << " + " << std::to_string(t.ys1[x]) << " = "
              << std::to_string(y) << std::endl;
    if (x == domain_slot) {
      if (y != 256) {
        exit_status = TEST_EXIT_FAIL;
      }
    } else {
      if (y != 0) {
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }
  return exit_status == TEST_EXIT_PASS;
}

template<class Scheme, int NumParties>
bool p_party_scheme_test_correctness(
    typename Scheme::rng_type & r,
    typename Scheme::domain_type domain_slot) {
  int exit_status = TEST_EXIT_PASS;
  p_party_test_helper<Scheme, NumParties> t;
  t.do_up_to_gen(domain_slot, 256, r);

  for (int i = 0; i < NumParties; i++) {
    t.deserialized_keys[i] = t.keys[i];
  }

  t.eval_all_deserialized();

  for (typename Scheme::domain_type x = 0; x < 2048; ++x) {
    typename Scheme::nice_range_type y = 0;
    for (int i = 0; i < NumParties; i++) {
      y ^= t.ys[i][x];
    }

    if (x == domain_slot) {
      if (y != 256) {
        exit_status = TEST_EXIT_FAIL;
      }
    } else {
      if (y != 0) {
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }
  return exit_status == TEST_EXIT_PASS;
}
