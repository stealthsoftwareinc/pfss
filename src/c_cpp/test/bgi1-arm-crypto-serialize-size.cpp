//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <exception>
#include <iostream>
#include <pfss/bgi1.h>
#include <pfss/common.h>
#include <pfss/scheme_test_serialization.h>

using namespace pfss;

int main() {
  try {
#if PFSS_HAVE_ARM_CRYPTO
    using block_type = uint8x16_block;
    using scheme_type = bgi1<
        20,
        9,
        unsigned long,
        unsigned long,
        block_type,
        arm_crypto_aes_128_rand_perm,
        cpp_urandom_rng<block_type>>;
    typename scheme_type::rng_type r;
    bool const pass = scheme_test_serialize_size<scheme_type>(r);
    return pass ? TEST_EXIT_PASS : TEST_EXIT_FAIL;
#else
    return TEST_EXIT_SKIP;
#endif
  } catch (std::exception const & e) {
    try {
      std::cout << e.what() << std::endl;
    } catch (...) {
    }
    return TEST_EXIT_ERROR;
  } catch (...) {
    return TEST_EXIT_ERROR;
  }
}
