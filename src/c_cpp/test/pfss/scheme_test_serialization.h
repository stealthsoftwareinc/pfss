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
  std::vector<typename Scheme::key_type> deserialized_keys;
  // initialize containers for range values. Each container is the
  // size of the full domain
  typename std::vector<std::vector<typename Scheme::nice_range_type>>
      ys;

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
};

template<class Scheme>
bool scheme_test_serialize_deserialize_key_members(
    typename Scheme::rng_type & r) {
  int exit_status = TEST_EXIT_PASS;
  test_helper<Scheme> t;
  t.do_up_to_gen(1031, 256, r);
  {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[0].serialize(blob.begin());
    t.deserialized_key0.parse(blob.cbegin());

    // compare member values
    if (t.deserialized_key0.domain_bits != t.keys[0].domain_bits
        || t.deserialized_key0.range_bits != t.keys[0].range_bits
        || t.deserialized_key0.party != t.keys[0].party
        || t.deserialized_key0.cw_last != t.keys[0].cw_last) {
      exit_status = TEST_EXIT_FAIL;
    }

    // compare s blocks
    for (size_t j = 0;
         j != static_cast<std::size_t>(t.keys[0].s.size());
         ++j) {
      if (t.deserialized_key0.s[j] != t.keys[0].s[j]) {
        exit_status = TEST_EXIT_FAIL;
      }
    }

    // compare cw's
    for (decltype(t.keys[0].cw.size()) i = 0; i != t.keys[0].cw.size();
         ++i) {
      for (size_t j = 0;
           j != static_cast<std::size_t>(t.keys[0].cw[i].scw.size());
           ++j) {
        if (t.deserialized_key0.cw[i].scw[j]
            != t.keys[0].cw[i].scw[j]) {
          exit_status = TEST_EXIT_FAIL;
        }
      }
      for (size_t j = 0; j < 2; ++j) {
        if (t.deserialized_key0.cw[i].tcw[j]
            != t.keys[0].cw[i].tcw[j]) {
          exit_status = TEST_EXIT_FAIL;
        }
      }
    }
  }
  return exit_status == TEST_EXIT_PASS;
}

template<class Scheme>
bool scheme_test_correctness_with_serialization(
    typename Scheme::rng_type & r) {
  int exit_status = TEST_EXIT_PASS;
  test_helper<Scheme> t;
  t.do_up_to_gen(1031, 256, r);

  {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[0].serialize(blob.begin());
    t.deserialized_key0.parse(blob.cbegin());
  }

  {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[1].serialize(blob.begin());
    t.deserialized_key1.parse(blob.cbegin());
  }

  t.eval_all_deserialized();

  for (typename Scheme::domain_type x = 1027; x < 1037; ++x) {
    auto const y = (t.ys0[x] + t.ys1[x]) & t.range_mask;
    std::cout << std::to_string(x) << ": " << std::to_string(t.ys0[x])
              << " + " << std::to_string(t.ys1[x]) << " = "
              << std::to_string(y) << std::endl;
    if (x == 1031) {
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

template<class Scheme>
bool scheme_test_correctness_with_serialization(
    typename Scheme::rng_type & r,
    typename Scheme::domain_type const & domain_slot) {
  int exit_status = TEST_EXIT_PASS;
  test_helper<Scheme> t;
  t.do_up_to_gen(domain_slot, 1, r);

  {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[0].serialize(blob.begin());
    t.deserialized_key0.parse(blob.cbegin());
  }

  {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[1].serialize(blob.begin());
    t.deserialized_key1.parse(blob.cbegin());
  }

  t.eval_all_deserialized();

  for (typename Scheme::domain_type x = 0; x < t.ys0.size(); ++x) {
    auto const y = (t.ys0[x] + t.ys1[x]) & t.range_mask;
    std::cout << std::to_string(x) << ": " << std::to_string(t.ys0[x])
              << " + " << std::to_string(t.ys1[x]) << " = "
              << std::to_string(y) << std::endl;
    if (x == domain_slot) {
      if (y != 1) {
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
bool p_party_scheme_test_correctness_with_serialization(
    typename Scheme::rng_type & r) {
  int exit_status = TEST_EXIT_PASS;
  p_party_test_helper<Scheme, NumParties> t;
  t.do_up_to_gen(1031, 321, r);
  for (int i = 0; i < NumParties; i++) {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[i].serialize(blob.begin());
    t.deserialized_keys[i].parse(blob.cbegin());
  }

  for (typename Scheme::domain_type x = 0; x < 2000; ++x) {
    typename Scheme::nice_range_type y = 0;
    for (int i = 0; i < NumParties; i++) {
      y ^= Scheme::eval(t.deserialized_keys[i], x, t.pi);
    }

    if (x == 1031) {
      if (y != 321) {
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

template<class Scheme>
bool scheme_test_serialize_size(typename Scheme::rng_type & r) {
  int exit_status = TEST_EXIT_PASS;
  test_helper<Scheme> t;
  t.do_up_to_gen(1031, 256, r);

  {
    std::vector<unsigned char> blob(Scheme::key_blob_size());
    t.keys[0].serialize(blob.begin());
    if (blob.size() != t.keys[0].size()) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  return exit_status == TEST_EXIT_PASS;
}
