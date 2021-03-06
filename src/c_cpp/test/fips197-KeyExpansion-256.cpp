//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <array>
#include <pfss/common.h>

using namespace pfss;

int main() {

  // FIPS 197 Appendix C.3
  {
    std::array<unsigned char, 32> const key = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    };
    std::array<unsigned char, 240> const correct_rkeys = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
        0x1E, 0x1F, 0xA5, 0x73, 0xC2, 0x9F, 0xA1, 0x76, 0xC4, 0x98,
        0xA9, 0x7F, 0xCE, 0x93, 0xA5, 0x72, 0xC0, 0x9C, 0x16, 0x51,
        0xA8, 0xCD, 0x02, 0x44, 0xBE, 0xDA, 0x1A, 0x5D, 0xA4, 0xC1,
        0x06, 0x40, 0xBA, 0xDE, 0xAE, 0x87, 0xDF, 0xF0, 0x0F, 0xF1,
        0x1B, 0x68, 0xA6, 0x8E, 0xD5, 0xFB, 0x03, 0xFC, 0x15, 0x67,
        0x6D, 0xE1, 0xF1, 0x48, 0x6F, 0xA5, 0x4F, 0x92, 0x75, 0xF8,
        0xEB, 0x53, 0x73, 0xB8, 0x51, 0x8D, 0xC6, 0x56, 0x82, 0x7F,
        0xC9, 0xA7, 0x99, 0x17, 0x6F, 0x29, 0x4C, 0xEC, 0x6C, 0xD5,
        0x59, 0x8B, 0x3D, 0xE2, 0x3A, 0x75, 0x52, 0x47, 0x75, 0xE7,
        0x27, 0xBF, 0x9E, 0xB4, 0x54, 0x07, 0xCF, 0x39, 0x0B, 0xDC,
        0x90, 0x5F, 0xC2, 0x7B, 0x09, 0x48, 0xAD, 0x52, 0x45, 0xA4,
        0xC1, 0x87, 0x1C, 0x2F, 0x45, 0xF5, 0xA6, 0x60, 0x17, 0xB2,
        0xD3, 0x87, 0x30, 0x0D, 0x4D, 0x33, 0x64, 0x0A, 0x82, 0x0A,
        0x7C, 0xCF, 0xF7, 0x1C, 0xBE, 0xB4, 0xFE, 0x54, 0x13, 0xE6,
        0xBB, 0xF0, 0xD2, 0x61, 0xA7, 0xDF, 0xF0, 0x1A, 0xFA, 0xFE,
        0xE7, 0xA8, 0x29, 0x79, 0xD7, 0xA5, 0x64, 0x4A, 0xB3, 0xAF,
        0xE6, 0x40, 0x25, 0x41, 0xFE, 0x71, 0x9B, 0xF5, 0x00, 0x25,
        0x88, 0x13, 0xBB, 0xD5, 0x5A, 0x72, 0x1C, 0x0A, 0x4E, 0x5A,
        0x66, 0x99, 0xA9, 0xF2, 0x4F, 0xE0, 0x7E, 0x57, 0x2B, 0xAA,
        0xCD, 0xF8, 0xCD, 0xEA, 0x24, 0xFC, 0x79, 0xCC, 0xBF, 0x09,
        0x79, 0xE9, 0x37, 0x1A, 0xC2, 0x3C, 0x6D, 0x68, 0xDE, 0x36,
    };
    std::array<unsigned char, 240> rkeys = {0};
    fips197::KeyExpansion<8>(key.cbegin(), rkeys.begin());
    if (rkeys != correct_rkeys) {
      return TEST_EXIT_FAIL;
    }
  }

  return TEST_EXIT_PASS;
}
