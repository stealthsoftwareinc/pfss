//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <exception>
#include <iostream>
#include <pfss/ds.h>
#include <pfss/common.h>
#include <pfss/scheme_test_1.h>

using namespace pfss;

int main() {
  try {
#if PFSS_WITH_NETTLE && PFSS_WITH_SSE2
    using block_type = m128i_block;
    using scheme_type = ds<
        20,
        9,
        unsigned long,
        unsigned long,
        block_type,
        rand_perm_nettle_aes128<block_type>,
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
