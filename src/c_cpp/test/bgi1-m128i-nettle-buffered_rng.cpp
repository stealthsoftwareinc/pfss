//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <exception>
#include <iostream>
#include <pfss/bgi1.h>
#include <pfss/common.h>
#include <pfss/scheme_test_2.h>

using namespace pfss;

int main() {
  try {
#if PFSS_HAVE_NETTLE && PFSS_HAVE_SSE2
    using block_type = m128i_block;
    using scheme_type = bgi1<
        20,
        9,
        unsigned long,
        unsigned long,
        block_type,
        rand_perm_nettle_aes128<block_type>,
        buffered_rng<block_type>>;
    std::vector<uint8_t> rand_buf(scheme_type::rand_buf_size());
    {
      uint32_t i;
      for (i = 0; i != rand_buf.size(); ++i) {
        rand_buf[i] = rand();
      }
    }
    bool const pass = scheme_test_2<scheme_type>(rand_buf.data());
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
