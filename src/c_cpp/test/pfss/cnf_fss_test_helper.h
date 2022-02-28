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
class cnf_fss_test_helper {
public:
  static constexpr int NumParties = 3;
  static constexpr int domain_bits = Scheme::domain_bits;
  static constexpr int range_bits = Scheme::range_bits;
  using DomainType = typename Scheme::domain_type;
  using RangeType = typename Scheme::range_type;
  using key_type = typename Scheme::key_type;
  std::array<key_type, NumParties> keys;
  std::array<key_type, NumParties> deserialized_keys;
  unsigned char aeskey[16] = {0};
  typename Scheme::rand_perm_type pi{aeskey};

  void do_up_to_gen(DomainType domain_slot,
                    RangeType range_value,
                    typename Scheme::rng_type & rng) {
    std::vector<unsigned char> control_bits_rand_buf(num_control_bits(range_bits));
    for (auto & byte : control_bits_rand_buf) {
      byte = std::rand();
    }

    std::vector<unsigned char> offset_rand_buf(num_rand_bytes_for_ms_dpf_plus(Scheme::lambda, domain_bits, range_bits));
    for (auto & byte : offset_rand_buf) {
      byte = std::rand();
    }

    Scheme::gen(keys,
            domain_slot,
            range_value,
            pi,
            rng,
            control_bits_rand_buf.begin(),
            offset_rand_buf.begin());
  }
};

template<class Scheme>
class cnf_fss_test_helper_v {
public:
  static constexpr int NumParties = 3;
  using DomainType = typename Scheme::domain_type;
  using RangeType = typename Scheme::range_type;
  using key_type = typename Scheme::key_type;
  unsigned char aeskey[16] = {0};
  typename Scheme::rand_perm_type pi{aeskey};

  void do_up_to_gen(std::array<key_type, NumParties> & keys,
                    int const & domain_bits,
                    int const & range_bits,
                    DomainType domain_slot,
                    RangeType range_value,
                    typename Scheme::rng_type & rng) {
    std::vector<unsigned char> control_bits_rand_buf(num_control_bits(range_bits));
    for (auto & byte : control_bits_rand_buf) {
      byte = std::rand();
    }

    std::vector<unsigned char> offset_rand_buf(num_rand_bytes_for_ms_dpf_plus(Scheme::lambda, domain_bits, range_bits));
    for (auto & byte : offset_rand_buf) {
      byte = std::rand();
    }

    Scheme cnf_fss(domain_bits, range_bits);
    cnf_fss.gen(
            keys,
            domain_slot,
            range_value,
            pi,
            rng,
            control_bits_rand_buf.begin(),
            offset_rand_buf.begin());
  }
};

template<class Scheme>
bool cnf_fss_test_correctness_with_serialization(
    typename Scheme::rng_type & r,
    typename Scheme::domain_type const & domain_slot,
    typename Scheme::range_type const & range_value) {
  int exit_status = TEST_EXIT_PASS;
  cnf_fss_test_helper<Scheme> t;
  t.do_up_to_gen(domain_slot, range_value, r);

  for (int i = 0; i < 3; i++) {
    std::vector<unsigned char> blob(Scheme::make_key().size());
    t.keys[i].serialize(blob.begin());
    t.deserialized_keys[i].parse(blob.cbegin());
    t.deserialized_keys[i].party = t.keys[i].party;
  }
  
  for (typename Scheme::domain_type x = 0; x < (static_cast<typename Scheme::domain_type>(1) << Scheme::domain_bits); ++x) {
    // if (x != domain_slot) continue;
    // std::cout << "\n\n\n" << x << std::endl;
    std::array<std::array<typename Scheme::range_type, 2>, 3> ys = {{{0, 0}, {0, 0}, {0, 0}}};
    typename Scheme::nice_range_type y = 0;
    for (int i = 0; i < t.NumParties; i++) {
      Scheme::eval(t.deserialized_keys[i], x, t.pi, ys[i]);
      y = static_cast<typename Scheme::nice_range_type>((y + ys[i][0]) % (1 << Scheme::range_bits));
    }
    // std::cout << "y: " << y << std::endl;
    if (x == domain_slot) {
      if (y != range_value) {
        std::cout << y << " != " << "range value, " << range_value << " at domain slot" << std::endl;
        exit_status = TEST_EXIT_FAIL;
      }
    } else {
      if (y != 0) {
        std::cout << y << " != 0 when alpha != domain slot" << std::endl;
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }

  return exit_status == TEST_EXIT_PASS;
}

template<class Scheme>
bool cnf_fss_test_correctness_with_serialization_v(
    int const & domain_bits,
    int const & range_bits,
    typename Scheme::rng_type & r,
    typename Scheme::domain_type const & domain_slot,
    typename Scheme::range_type const & range_value) {
  int exit_status = TEST_EXIT_PASS;
  cnf_fss_test_helper_v<Scheme> t;

  std::array<typename Scheme::key_type, 3> keys = 
      {typename Scheme::key_type(domain_bits, range_bits),
      typename Scheme::key_type(domain_bits, range_bits),
      typename Scheme::key_type(domain_bits, range_bits)};
  std::array<typename Scheme::key_type, 3> deserialized_keys = 
      {typename Scheme::key_type(domain_bits, range_bits),
      typename Scheme::key_type(domain_bits, range_bits),
      typename Scheme::key_type(domain_bits, range_bits)};
  t.do_up_to_gen(keys, domain_bits, range_bits, domain_slot, range_value, r);

  for (int i = 0; i < 3; i++) {
    Scheme fss(domain_bits, range_bits);
    std::vector<unsigned char> blob(fss.make_key().size());
    keys[i].serialize(blob.begin());
    deserialized_keys[i].parse(blob.cbegin());
    deserialized_keys[i].party = keys[i].party;
  }
  
  for (typename Scheme::domain_type x = 0; x < (static_cast<typename Scheme::domain_type>(1) << domain_bits); ++x) {
    // if (x != domain_slot) continue;
    // std::cout << "\n\n\n" << x << std::endl;
    std::array<std::array<typename Scheme::range_type, 2>, 3> ys = {{{0, 0}, {0, 0}, {0, 0}}};
    typename Scheme::nice_range_type y = 0;
    for (int i = 0; i < t.NumParties; i++) {
      Scheme fss(domain_bits, range_bits);
      fss.eval(deserialized_keys[i], x, t.pi, ys[i]);
      y = static_cast<typename Scheme::nice_range_type>((y + ys[i][0]) % (1 << range_bits));
    }
    // std::cout << "y: " << y << std::endl;
    if (x == domain_slot) {
      if (y != range_value) {
        std::cout << y << " != " << "range value, " << range_value << " at domain slot" << std::endl;
        exit_status = TEST_EXIT_FAIL;
      }
    } else {
      if (y != 0) {
        std::cout << y << " != 0 when alpha != domain slot" << std::endl;
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }

  return exit_status == TEST_EXIT_PASS;
}
