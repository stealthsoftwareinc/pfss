/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <cpp_helpers.h>

using namespace pfss;

// test the correctness of the overall execution of the bgi functions:
// gen --> serialize keys --> parse serialized keys --> eval
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
  }

  {
    std::vector<unsigned char> blob(scheme_type::key_blob_size());
    t.keys[1].serialize(blob.begin());
    t.deserialized_key1.parse(blob.cbegin());
  }


  t.eval_all_deserialized();

  for (DomainType x = 1027; x < 1037; ++x) {
    auto const y = (t.ys0[x] + t.ys1[x]) & t.range_mask;
    std::cout << std::to_string(x) << ": " << std::to_string(t.ys0[x])
              << " + " << std::to_string(t.ys1[x]) << " = "
              << std::to_string(y)
              << std::endl;
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

  return exit_status;
}
