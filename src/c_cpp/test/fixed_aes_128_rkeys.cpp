//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <array>
#include <pfss/common.h>

using namespace pfss;

int main() {
  std::array<unsigned char, 16> const key =
#include <pfss/fixed_aes_128_key.inc>
      ;
  std::array<unsigned char, 176> const rkeys = {
#include <pfss/fixed_aes_128_rkeys.inc>
  };
  std::array<unsigned char, 176> correct_rkeys;
  fips197::KeyExpansion<4>(key.cbegin(), correct_rkeys.begin());
  return rkeys == correct_rkeys ? TEST_EXIT_PASS : TEST_EXIT_FAIL;
}
