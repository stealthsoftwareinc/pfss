//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <exception>
#include <iostream>
#include <pfss/bgi_p_way.h>
#include <pfss/common.h>
#include <pfss/scheme_test_eval_all.h>

using namespace pfss;

int main() {
  static constexpr int num_parties = 5;
  try {
#if PFSS_WITH_AES_NI
    using block_type = m128i_block;
    using scheme_type = bgip<num_parties,
                             20,
                             9,
                             unsigned long,
                             unsigned long,
                             std::vector<unsigned char>::iterator,
                             block_type,
                             aes_ni_128_rand_perm<false>,
                             cpp_urandom_rng<block_type>>;
    typename scheme_type::rng_type r;
    bool const pass =
        p_party_scheme_test_correctness<scheme_type, num_parties>(r,
                                                                  1031);
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
