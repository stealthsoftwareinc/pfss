//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <pfss/common.h>

namespace pfss {

template<class Scheme>
bool scheme_test_1() {
  typename Scheme::key_type keys[2];
  typename Scheme::domain_type const alpha = 1031;
  typename Scheme::nice_range_type const beta = 256;
  typename Scheme::rand_perm_type pi(fixed_aes_128_key.data());
  typename Scheme::rng_type rng;
  Scheme::gen(keys, alpha, beta, pi, rng);
  typename Scheme::domain_type x = 0;
  typename Scheme::nice_range_type sum = 0;
  do {
    sum += Scheme::eval(keys[0], x, pi);
    sum += Scheme::eval(keys[1], x, pi);
  } while (++x != get_mask<decltype(x)>(Scheme::domain_bits));
  return (sum & Scheme::range_mask) == beta;
}

} // namespace pfss
