//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <array>
#include <iostream>
#include <pfss/common.h>
#include <stdexcept>
#include <string>

namespace {

struct fail : std::runtime_error {
  explicit fail(std::string const & what_arg) :
      std::runtime_error(what_arg) {
  }
  explicit fail(char const * const what_arg) :
      std::runtime_error(what_arg) {
  }
};

struct error : std::runtime_error {
  explicit error(std::string const & what_arg) :
      std::runtime_error(what_arg) {
  }
  explicit error(char const * const what_arg) :
      std::runtime_error(what_arg) {
  }
};

template<class Block, class RandPerm>
struct tester final {

  using uchar = unsigned char;

  using mimic = std::array<uchar, Block::bytes>;

  std::string name;

  // clang-format off

  // The following test vectors (FIPS 197) were taken from:
  // https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197.pdf
  // The code below will reference this source as FIPS 197

  // Test vectors from FIPS 197 Appendix C.1
  mimic fips197c1_key = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  mimic fips197c1_plaintext = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
  };
  mimic fips197c1_output = {
    0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 
    0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a,
  };

  // Test vectors from FIPS 197 Appendix B
  mimic fips197b_key = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
  };
  mimic fips197b_plaintext = {
    0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 
    0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,
  };
  mimic fips197b_output = {
    0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 
    0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32,
  };


  // The following test vectors (NIST SP 800-38A) were taken from:
  // https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38a.pdf
  // The code below will reference this source as NIST SP 800-38A

  // Test vectors from NIST SP 800-38A Appendix F.1.1, Block #1
  mimic nist_sp_800_38a_f111_key = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
  };  
  mimic nist_sp_800_38a_f111_plaintext = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
  };
  mimic nist_sp_800_38a_f111_output = {
    0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 
    0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97,
  };

  // Test vectors from NIST SP 800-38A Appendix F.1.1, Block #2
  // (uses the same key as Block #1)
  mimic nist_sp_800_38a_f112_key = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
  };  
  mimic nist_sp_800_38a_f112_plaintext = {
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
  };
  mimic nist_sp_800_38a_f112_output = {
    0xf5, 0xd3, 0xd5, 0x85, 0x03, 0xb9, 0x69, 0x9d, 
    0xe7, 0x85, 0x89, 0x5a, 0x96, 0xfd, 0xba, 0xaf,
  };

  // Test vectors from NIST SP 800-38A Appendix F.1.1, Block #3
  // (uses the same key as Block #1)
  mimic nist_sp_800_38a_f113_key = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
  };  
  mimic nist_sp_800_38a_f113_plaintext = {
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 
    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
  };
  mimic nist_sp_800_38a_f113_output = {
    0x43, 0xb1, 0xcd, 0x7f, 0x59, 0x8e, 0xce, 0x23, 
    0x88, 0x1b, 0x00, 0xe3, 0xed, 0x03, 0x06, 0x88,
  };

  // Test vectors from NIST SP 800-38A Appendix F.1.1, Block #4
  // (uses the same key as Block #1)
  mimic nist_sp_800_38a_f114_key = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
  };  
  mimic nist_sp_800_38a_f114_plaintext = {
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 
    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10,
  };
  mimic nist_sp_800_38a_f114_output = {
    0x7b, 0x0c, 0x78, 0x5e, 0x27, 0xe8, 0xad, 0x3f, 
    0x82, 0x23, 0x20, 0x71, 0x04, 0x72, 0x5d, 0xd4,
  };

  // clang-format on
  tester(std::string const & name) : name(name) {
  }

  void test() {
    mimic m1;
    Block b1;

#define TEST_VECTOR(x) \
  do { \
    RandPerm p(x##_key.data()); \
    b1.parse(x##_plaintext.cbegin()); \
    b1 = p(b1); \
    b1.serialize(m1.begin()); \
    if (m1 != x##_output) { \
      throw fail( \
          name + " " #x \
          " failed: returned output does not match expected output"); \
    } \
  } while (0)

    TEST_VECTOR(fips197c1);
    TEST_VECTOR(fips197b);
    TEST_VECTOR(nist_sp_800_38a_f111);
    TEST_VECTOR(nist_sp_800_38a_f112);
    TEST_VECTOR(nist_sp_800_38a_f113);
    TEST_VECTOR(nist_sp_800_38a_f114);
    
    std::cout << "pass: " << name << std::endl;
  }
};

// This lets us pass macro arguments with commas in them.
#define UNWRAP(...) __VA_ARGS__
std::string unwrap(char const * const s) {
  std::string x(s);
  return x.substr(1, x.size() - 2);
}

#define TEST(Block, RandPerm) \
  tester<UNWRAP Block, UNWRAP RandPerm>( \
      unwrap(#Block) + " + " + unwrap(#RandPerm)) \
      .test()
#define SKIP(Block, RandPerm) \
  (std::cout << "skip: " << unwrap(#Block) << " + " \
             << unwrap(#RandPerm) << std::endl)

} // namespace

int main() {
  try {

#if PFSS_WITH_NETTLE
    TEST(
        (pfss::chunked_block<std::uint64_t, 2>),
        (pfss::rand_perm_nettle_aes128<
            pfss::chunked_block<std::uint64_t, 2>>));
#else
    SKIP(
        (pfss::chunked_block<std::uint64_t, 2>),
        (pfss::rand_perm_nettle_aes128<
            pfss::chunked_block<std::uint64_t, 2>>));
#endif

#if PFSS_WITH_NETTLE && PFSS_WITH_SSE2
    TEST(
        (pfss::m128i_block),
        (pfss::rand_perm_nettle_aes128<pfss::m128i_block>));
#else
    SKIP(
        (pfss::m128i_block),
        (pfss::rand_perm_nettle_aes128<pfss::m128i_block>));
#endif

#if PFSS_WITH_AES_NI
    TEST((pfss::m128i_block), (pfss::aes_ni_128_rand_perm<>));
#else
    SKIP((pfss::m128i_block), (pfss::aes_ni_128_rand_perm<>));
#endif

#if PFSS_WITH_ARM_CRYPTO
    TEST((pfss::uint8x16_block), (pfss::arm_crypto_aes_128_rand_perm));
#else
    SKIP((pfss::uint8x16_block), (pfss::arm_crypto_aes_128_rand_perm));
#endif

  } catch (fail const & e) {
    try {
      std::cout << e.what() << std::endl;
    } catch (...) {
    }
    return TEST_EXIT_FAIL;
  } catch (std::exception const & e) {
    try {
      std::cout << e.what() << std::endl;
    } catch (...) {
    }
    return TEST_EXIT_ERROR;
  } catch (...) {
    return TEST_EXIT_ERROR;
  }
  return TEST_EXIT_PASS;
}
