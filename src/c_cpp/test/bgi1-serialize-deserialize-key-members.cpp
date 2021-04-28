/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <cpp_helpers.h>

using namespace pfss;

// the # bytes contained in the serialization should match the # bytes
// returned by key_type's size() function
int main() {
  int exit_status = TEST_EXIT_PASS;
  constexpr size_t DomainBits = 20;
  constexpr size_t RangeBits = 9;
  using DomainType = unsigned long long;
  using RangeType = uint16_t;
  
  using test_type = bgi1_test<DomainBits, RangeBits, DomainType, RangeType>;
  using scheme_type = test_type::Bgi1;
  test_type t;
  t.do_up_to_gen(1031, 256);
  {
    std::vector<unsigned char> blob(scheme_type::key_blob_size());
    t.keys[0].serialize(blob.begin());
    t.deserialized_key0.parse(blob.cbegin());

    // compare member values
    if (t.deserialized_key0.domain_bits != t.keys[0].domain_bits
      || t.deserialized_key0.range_bits != t.keys[0].range_bits
      || t.deserialized_key0.party != t.keys[0].party
      || t.deserialized_key0.cw_last != t.keys[0].cw_last
      ) {
      exit_status = TEST_EXIT_FAIL;
    }

    // compare s blocks
    for (size_t j = 0; j != static_cast<std::size_t>(t.keys[0].s.size()); ++j) {
      if (t.deserialized_key0.s[j] != t.keys[0].s[j]) {
        exit_status = TEST_EXIT_FAIL;
      }
    }

    // compare cw's
    for (size_t i = 0; i != t.keys[0].cw.size(); ++i) {
      for (size_t j = 0; j != static_cast<std::size_t>(t.keys[0].cw[i].scw.size()); ++j) {
        if (t.deserialized_key0.cw[i].scw[j] != t.keys[0].cw[i].scw[j]) {
          exit_status = TEST_EXIT_FAIL;
        }
      }
      for (size_t j = 0; j < 2; ++j) {
        if (t.deserialized_key0.cw[i].tcw[j] != t.keys[0].cw[i].tcw[j]) {
          exit_status = TEST_EXIT_FAIL;
        }
      }
    }
  }

  return exit_status;
}
