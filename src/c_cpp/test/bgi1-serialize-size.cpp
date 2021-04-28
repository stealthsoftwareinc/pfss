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
    if (blob.size() != t.keys[0].size()) {
    	exit_status = TEST_EXIT_FAIL;
    }
  }
  return exit_status;
}
