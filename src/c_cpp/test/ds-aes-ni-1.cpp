//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <exception>
#include <iostream>
#include <pfss/common.h>
#include <pfss/ds.h>
#include <pfss/scheme_test_1.h>

using namespace pfss;

int main() {
  try {
#if PFSS_WITH_AES_NI
    using block_type = m128i_block;
    using scheme_type =
        ds<20,
           9,
           unsigned long,
           unsigned long,
           block_type,
           aes_ni_128_rand_perm<false>,
           cpp_urandom_rng<block_type>>;
    bool const pass = scheme_test_1<scheme_type>();
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
