/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <iostream>
#include <pfss/common.h>
using namespace pfss;
int main() {
  uint16_t const x = 0xFFFF;
  uint16_t const y = 0;
  uint16_t const max_bits = 64;
  
  uint_buf <>b0(
        (uint8_t const *)&x,
        sizeof(x),
        fold_endianness(PFSS_NATIVE_ENDIAN));

  uint_buf <>b1(
        (uint8_t const *)&y,
        sizeof(y),
        fold_endianness(PFSS_NATIVE_ENDIAN));

  b1 = b0;

  if (b1 == b0 && b1.get_as<uint16_t>() == 0xFFFF) {
    return TEST_EXIT_PASS;
  } else { return TEST_EXIT_FAIL; }
}
