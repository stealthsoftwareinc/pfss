/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>
#include <pfss/config.h>
#include <stdint.h>
#include <stdio.h>

#if PFSS_WITH_AES_NI

#define DEFINE_TEST_HELPER(domain_bits, range_type) \
\
  PFSS_DEFINE_DIRECT_EVAL( \
      eval_##domain_bits##_##range_type, (domain_bits), range_type) \
\
  static int test_##domain_bits##_##range_type(void) { \
    DO_UP_TO_INCLUDING_GEN( \
        (domain_bits), (sizeof(range_type) * 8), 1, 234); \
    { \
      uint64_t x; \
      for (x = 0; x != 1000; ++x) { \
        uint8_t b[(domain_bits) / 8 + ((domain_bits) % 8 != 0)]; \
        uint64_t bx = x; \
        size_t bi; \
        range_type y1; \
        range_type y2; \
        range_type sum; \
        uint64_t x_chop; \
        for (bi = 0; bi != sizeof(b); ++bi) { \
          b[bi] = bx; \
          bx >>= 4; \
          bx >>= 4; \
        } \
        y1 = eval_##domain_bits##_##range_type(key1_blob, b); \
        y2 = eval_##domain_bits##_##range_type(key2_blob, b); \
        sum = (y1 + 0U + y2); \
        if ((domain_bits) > 63) { \
          x_chop = x; \
        } else { \
          x_chop = x & ((1ULL << ((domain_bits)&63)) - 1); \
        } \
        if (x_chop == domain_value) { \
          if (sum != range_value) { \
            printf( \
                "eval_%d_" #range_type \
                "(key1_blob, %llu) + eval_%d_" #range_type \
                "(key2_blob, %llu) = %llu + %llu = %llu != %llu " \
                "(mod " #range_type ")\n", \
                (int)(domain_bits), \
                (unsigned long long)x_chop, \
                (int)(domain_bits), \
                (unsigned long long)x_chop, \
                (unsigned long long)y1, \
                (unsigned long long)y2, \
                (unsigned long long)sum, \
                (unsigned long long)range_value); \
            exit_status = TEST_EXIT_FAIL; \
          } \
        } else { \
          if (sum != 0) { \
            printf( \
                "eval_%d_" #range_type \
                "(key1_blob, %llu) + eval_%d_" #range_type \
                "(key2_blob, %llu) = %llu + %llu = %llu != 0 " \
                "(mod " #range_type ")\n", \
                (int)(domain_bits), \
                (unsigned long long)x_chop, \
                (int)(domain_bits), \
                (unsigned long long)x_chop, \
                (unsigned long long)y1, \
                (unsigned long long)y2, \
                (unsigned long long)sum); \
            exit_status = TEST_EXIT_FAIL; \
          } \
        } \
      } \
    } \
    goto cleanup; \
  }

#define DEFINE_TEST(domain_bits) \
  DEFINE_TEST_HELPER(domain_bits, uint8_t) \
  DEFINE_TEST_HELPER(domain_bits, uint16_t) \
  DEFINE_TEST_HELPER(domain_bits, uint32_t) \
  DEFINE_TEST_HELPER(domain_bits, uint64_t)

DEFINE_TEST(1)
DEFINE_TEST(2)
DEFINE_TEST(3)
DEFINE_TEST(4)
DEFINE_TEST(5)
DEFINE_TEST(6)
DEFINE_TEST(7)
DEFINE_TEST(8)
DEFINE_TEST(9)
DEFINE_TEST(10)
DEFINE_TEST(11)
DEFINE_TEST(12)
DEFINE_TEST(13)
DEFINE_TEST(14)
DEFINE_TEST(15)
DEFINE_TEST(16)
DEFINE_TEST(17)
DEFINE_TEST(18)
DEFINE_TEST(19)
DEFINE_TEST(20)
DEFINE_TEST(21)
DEFINE_TEST(22)
DEFINE_TEST(23)
DEFINE_TEST(24)
DEFINE_TEST(25)
DEFINE_TEST(26)
DEFINE_TEST(27)
DEFINE_TEST(28)
DEFINE_TEST(29)
DEFINE_TEST(30)
DEFINE_TEST(31)
DEFINE_TEST(32)
DEFINE_TEST(33)
DEFINE_TEST(34)
DEFINE_TEST(35)
DEFINE_TEST(36)
DEFINE_TEST(37)
DEFINE_TEST(38)
DEFINE_TEST(39)
DEFINE_TEST(40)
DEFINE_TEST(41)
DEFINE_TEST(42)
DEFINE_TEST(43)
DEFINE_TEST(44)
DEFINE_TEST(45)
DEFINE_TEST(46)
DEFINE_TEST(47)
DEFINE_TEST(48)
DEFINE_TEST(49)
DEFINE_TEST(50)
DEFINE_TEST(51)
DEFINE_TEST(52)
DEFINE_TEST(53)
DEFINE_TEST(54)
DEFINE_TEST(55)
DEFINE_TEST(56)
DEFINE_TEST(57)
DEFINE_TEST(58)
DEFINE_TEST(59)
DEFINE_TEST(60)
DEFINE_TEST(61)
DEFINE_TEST(62)
DEFINE_TEST(63)
DEFINE_TEST(64)
DEFINE_TEST(65)
DEFINE_TEST(66)
DEFINE_TEST(67)
DEFINE_TEST(68)
DEFINE_TEST(69)
DEFINE_TEST(70)
DEFINE_TEST(71)
DEFINE_TEST(72)
DEFINE_TEST(73)
DEFINE_TEST(74)
DEFINE_TEST(75)
DEFINE_TEST(76)
DEFINE_TEST(77)
DEFINE_TEST(78)
DEFINE_TEST(79)
DEFINE_TEST(80)
DEFINE_TEST(81)
DEFINE_TEST(82)
DEFINE_TEST(83)
DEFINE_TEST(84)
DEFINE_TEST(85)
DEFINE_TEST(86)
DEFINE_TEST(87)
DEFINE_TEST(88)
DEFINE_TEST(89)
DEFINE_TEST(90)
DEFINE_TEST(91)
DEFINE_TEST(92)
DEFINE_TEST(93)
DEFINE_TEST(94)
DEFINE_TEST(95)
DEFINE_TEST(96)
DEFINE_TEST(97)
DEFINE_TEST(98)
DEFINE_TEST(99)
DEFINE_TEST(100)
DEFINE_TEST(101)
DEFINE_TEST(102)
DEFINE_TEST(103)
DEFINE_TEST(104)
DEFINE_TEST(105)
DEFINE_TEST(106)
DEFINE_TEST(107)
DEFINE_TEST(108)
DEFINE_TEST(109)
DEFINE_TEST(110)
DEFINE_TEST(111)
DEFINE_TEST(112)
DEFINE_TEST(113)
DEFINE_TEST(114)
DEFINE_TEST(115)
DEFINE_TEST(116)
DEFINE_TEST(117)
DEFINE_TEST(118)
DEFINE_TEST(119)
DEFINE_TEST(120)
DEFINE_TEST(121)
DEFINE_TEST(122)
DEFINE_TEST(123)
DEFINE_TEST(124)
DEFINE_TEST(125)
DEFINE_TEST(126)
DEFINE_TEST(127)
DEFINE_TEST(128)

#define TEST_HELPER(domain_bits, range_type) \
  do { \
    int const s = test_##domain_bits##_##range_type(); \
    if (s == TEST_EXIT_ERROR) { \
      return s; \
    } else if (s == TEST_EXIT_FAIL) { \
      exit_status = TEST_EXIT_FAIL; \
    } \
  } while (0)

#define TEST(domain_bits) \
  do { \
    TEST_HELPER(domain_bits, uint8_t); \
    TEST_HELPER(domain_bits, uint16_t); \
    TEST_HELPER(domain_bits, uint32_t); \
    TEST_HELPER(domain_bits, uint64_t); \
  } while (0)

int main(void) {
  int exit_status = EXIT_SUCCESS;
  TEST(1);
  TEST(2);
  TEST(3);
  TEST(4);
  TEST(5);
  TEST(6);
  TEST(7);
  TEST(8);
  TEST(9);
  TEST(10);
  TEST(11);
  TEST(12);
  TEST(13);
  TEST(14);
  TEST(15);
  TEST(16);
  TEST(17);
  TEST(18);
  TEST(19);
  TEST(20);
  TEST(21);
  TEST(22);
  TEST(23);
  TEST(24);
  TEST(25);
  TEST(26);
  TEST(27);
  TEST(28);
  TEST(29);
  TEST(30);
  TEST(31);
  TEST(32);
  TEST(33);
  TEST(34);
  TEST(35);
  TEST(36);
  TEST(37);
  TEST(38);
  TEST(39);
  TEST(40);
  TEST(41);
  TEST(42);
  TEST(43);
  TEST(44);
  TEST(45);
  TEST(46);
  TEST(47);
  TEST(48);
  TEST(49);
  TEST(50);
  TEST(51);
  TEST(52);
  TEST(53);
  TEST(54);
  TEST(55);
  TEST(56);
  TEST(57);
  TEST(58);
  TEST(59);
  TEST(60);
  TEST(61);
  TEST(62);
  TEST(63);
  TEST(64);
  TEST(65);
  TEST(66);
  TEST(67);
  TEST(68);
  TEST(69);
  TEST(70);
  TEST(71);
  TEST(72);
  TEST(73);
  TEST(74);
  TEST(75);
  TEST(76);
  TEST(77);
  TEST(78);
  TEST(79);
  TEST(80);
  TEST(81);
  TEST(82);
  TEST(83);
  TEST(84);
  TEST(85);
  TEST(86);
  TEST(87);
  TEST(88);
  TEST(89);
  TEST(90);
  TEST(91);
  TEST(92);
  TEST(93);
  TEST(94);
  TEST(95);
  TEST(96);
  TEST(97);
  TEST(98);
  TEST(99);
  TEST(100);
  TEST(101);
  TEST(102);
  TEST(103);
  TEST(104);
  TEST(105);
  TEST(106);
  TEST(107);
  TEST(108);
  TEST(109);
  TEST(110);
  TEST(111);
  TEST(112);
  TEST(113);
  TEST(114);
  TEST(115);
  TEST(116);
  TEST(117);
  TEST(118);
  TEST(119);
  TEST(120);
  TEST(121);
  TEST(122);
  TEST(123);
  TEST(124);
  TEST(125);
  TEST(126);
  TEST(127);
  TEST(128);
  return exit_status;
}

#else

int main(void) {
  return TEST_EXIT_SKIP;
}

#endif
