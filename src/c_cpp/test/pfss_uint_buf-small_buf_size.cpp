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
  uint_buf<> b(
        (uint8_t const *)&x,
        1, // set the buffer size to be half the # of bytes in x
        fold_endianness(PFSS_NATIVE_ENDIAN));

  if (b.get_as<uint16_t>() == 0xFF) {
    return TEST_EXIT_PASS;
  } else { return TEST_EXIT_FAIL; }
}
