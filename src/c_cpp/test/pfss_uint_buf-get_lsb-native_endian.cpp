/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <iostream>
#include <pfss/common.h>
using namespace pfss;
int main() {
  uint32_t const x = 1;
  uint_buf<> b(
        (uint8_t const *)&x,
        sizeof(x),
        fold_endianness(PFSS_NATIVE_ENDIAN));

  if (b.getbitx(0)) {
    return TEST_EXIT_PASS;
  } else { return TEST_EXIT_FAIL; }
}
