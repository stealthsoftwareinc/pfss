//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <exception>
#include <iostream>
#include <pfss/bko_1-3_cnf_fss.h>
#include <pfss/bgi1_z2n_group.h>
#include <pfss/bko_ms_dpf_plus.h>
#include <pfss/common.h>
#include <pfss/cnf_fss_test_helper.h>

using namespace pfss;

int main() {
  try {
#if PFSS_WITH_AES_NI
    using block_type = m128i_block;
    using inner_scheme_type = bgi1_z2n_v<unsigned long,
                                         z2n_v,
                                         block_type,
                                         aes_ni_128_rand_perm<false>,
                                         cpp_urandom_rng<block_type>>;
    using two_party_ms_dpf_plus_type =
        bko_two_party_ms_dpf_plus_v<inner_scheme_type>;
    using three_party_ms_dpf_plus_type = bko_ms_dpf_plus_v<two_party_ms_dpf_plus_type>;
    using scheme_type = bko_1_3_cnf_fss<
        10,
        4,
        unsigned long,
        unsigned long,
        three_party_ms_dpf_plus_type,
        std::vector<unsigned char>::iterator>;
    typename scheme_type::rng_type r;
    bool const pass =
        cnf_fss_test_correctness_with_serialization<scheme_type>(
            r,
            1001,
            10);
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
