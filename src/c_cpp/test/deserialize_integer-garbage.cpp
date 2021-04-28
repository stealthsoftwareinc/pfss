/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <iostream>
#include <pfss.h>
#include <pfss/bgi1.h>
#include <string>

using namespace pfss;

// deserialization should throw an error if it is passed a buffer
// containing ones in the zero-padding area
int main() {
  constexpr size_t RangeBits = 9;
  using RangeType = uint16_t;
  constexpr int nq = RangeBits / uchar_bits;
  constexpr int nr = RangeBits % uchar_bits;
  constexpr int n = nq + (nr != 0);
  std::vector<unsigned char> blob(n, 0xFF); // fill serialized buffer with bogus values
  RangeType deserialized_val = 0;
  try {
    deserialize_integer(RangeBits, deserialized_val, blob.cbegin());
  } catch (...) {
    return TEST_EXIT_PASS;
  }

  return TEST_EXIT_FAIL;
}
