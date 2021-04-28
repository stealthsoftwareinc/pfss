//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <algorithm>
#include <cstdint>
#include <pfss.h>

namespace {

struct error {};
struct fail {};

template<int RangeBits, class Y, class Z>
void test_1() {
  // Aligned.
  Y const ys[] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
  {
    Z z = 0;
    pfss_status const status = pfss_reduce_sum(
        RangeBits,
        reinterpret_cast<std::uint8_t const *>(ys),
        sizeof(ys) / sizeof(*ys),
        sizeof(Y),
        PFSS_NATIVE_ENDIAN,
        reinterpret_cast<std::uint8_t *>(&z),
        sizeof(Z),
        PFSS_NATIVE_ENDIAN);
    if (status != PFSS_OK) {
      throw error();
    }
    if (z != (253 & (RangeBits >= 8 ? 255 : (1U << RangeBits) - 1))) {
      throw fail();
    }
  }
  // Unaligned.
  std::uint8_t ys2[1 + sizeof(ys)];
  std::copy_n(
      reinterpret_cast<std::uint8_t const *>(ys), sizeof(ys), ys2 + 1);
  {
    Z z = 0;
    pfss_status const status = pfss_reduce_sum(
        RangeBits,
        ys2 + 1,
        sizeof(ys) / sizeof(*ys),
        sizeof(Y),
        PFSS_NATIVE_ENDIAN,
        reinterpret_cast<std::uint8_t *>(&z),
        sizeof(Z),
        PFSS_NATIVE_ENDIAN);
    if (status != PFSS_OK) {
      throw error();
    }
    if (z != (253 & (RangeBits >= 8 ? 255 : (1U << RangeBits) - 1))) {
      throw fail();
    }
  }
}

} // namespace

int main() {
  try {

#define X_1_TO_8(F, N, Y, Z) \
  do { \
    F<1 + (N), Y, Z>(); \
    F<2 + (N), Y, Z>(); \
    F<3 + (N), Y, Z>(); \
    F<4 + (N), Y, Z>(); \
    F<5 + (N), Y, Z>(); \
    F<6 + (N), Y, Z>(); \
    F<7 + (N), Y, Z>(); \
    F<8 + (N), Y, Z>(); \
  } while (false)

#define X_1_TO_16(F, N, Y, Z) \
  do { \
    X_1_TO_8(F, 0 + (N), Y, Z); \
    X_1_TO_8(F, 8 + (N), Y, Z); \
  } while (false)

#define X_1_TO_32(F, N, Y, Z) \
  do { \
    X_1_TO_16(F, 0 + (N), Y, Z); \
    X_1_TO_16(F, 16 + (N), Y, Z); \
  } while (false)

#define X_1_TO_64(F, N, Y, Z) \
  do { \
    X_1_TO_32(F, 0 + (N), Y, Z); \
    X_1_TO_32(F, 32 + (N), Y, Z); \
  } while (false)

#define X_ALL(F) \
  do { \
    X_1_TO_8(test_1, 0, std::uint8_t, std::uint8_t); \
    X_1_TO_8(test_1, 0, std::uint8_t, std::uint16_t); \
    X_1_TO_8(test_1, 0, std::uint8_t, std::uint32_t); \
    X_1_TO_8(test_1, 0, std::uint8_t, std::uint64_t); \
    X_1_TO_8(test_1, 0, std::uint16_t, std::uint8_t); \
    X_1_TO_16(test_1, 0, std::uint16_t, std::uint16_t); \
    X_1_TO_16(test_1, 0, std::uint16_t, std::uint32_t); \
    X_1_TO_16(test_1, 0, std::uint16_t, std::uint64_t); \
    X_1_TO_8(test_1, 0, std::uint32_t, std::uint8_t); \
    X_1_TO_16(test_1, 0, std::uint32_t, std::uint16_t); \
    X_1_TO_32(test_1, 0, std::uint32_t, std::uint32_t); \
    X_1_TO_32(test_1, 0, std::uint32_t, std::uint64_t); \
    X_1_TO_8(test_1, 0, std::uint64_t, std::uint8_t); \
    X_1_TO_16(test_1, 0, std::uint64_t, std::uint16_t); \
    X_1_TO_32(test_1, 0, std::uint64_t, std::uint32_t); \
    X_1_TO_64(test_1, 0, std::uint64_t, std::uint64_t); \
  } while (false)

    X_ALL(test_1);

  } catch (fail const &) {
    return TEST_EXIT_FAIL;
  } catch (...) {
    return TEST_EXIT_ERROR;
  }
  return TEST_EXIT_PASS;
}
