/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <iostream>
#include <pfss.h>
#include <pfss/bgi1.h>
#include <pfss/common.h>
#include <pfss/config.h>
#include <string>

using namespace pfss;

#if PFSS_HAVE_AES_NI
using block_type = m128i_block;
using rand_perm_type = aes_ni_128_rand_perm<>;
#elif PFSS_HAVE_ARM_CRYPTO
using block_type = uint8x16_block;
using rand_perm_type = arm_crypto_aes_128_rand_perm;
#elif PFSS_HAVE_SSE2
using block_type = m128i_block;
using rand_perm_type = rand_perm_nettle_aes128<block_type>;
#else
using block_type = chunked_block<uint64_t, 2>;
using rand_perm_type = rand_perm_nettle_aes128<block_type>;
#endif

template<
    size_t DomainBits,
    size_t RangeBits,
    class DomainType,
    class RangeType,
    class Block = block_type,
    class RandPerm = rand_perm_type,
    class Rng = cpp_urandom_rng<block_type>>
class bgi1_test {
public:
  using Bgi1 = bgi1<DomainBits, RangeBits, DomainType, RangeType, Block, RandPerm, Rng>;
  static constexpr DomainType domain_mask =
      get_mask<DomainType>(DomainBits);
  static constexpr typename Bgi1::nice_range_type range_mask =
      get_mask<RangeType>(RangeBits);

  typename Bgi1::key_type keys[2];
  typename std::vector<typename Bgi1::nice_range_type> ys0 = decltype(ys0)(domain_mask + 1, 0);
  typename std::vector<typename Bgi1::nice_range_type> ys1 = decltype(ys1)(domain_mask + 1, 0);
  typename Bgi1::key_type deserialized_key0;
  typename Bgi1::key_type deserialized_key1;

  unsigned char aeskey[16] = {0};
  RandPerm pi{aeskey};
  Rng rng;

  void do_up_to_gen(
    DomainType domain_slot, 
    RangeType range_value 
  ) {
    Bgi1::gen(keys, domain_slot, range_value, pi, rng);
  }

  void eval_all_deserialized() {
    Bgi1::eval_all(deserialized_key0, ys0, pi);
    Bgi1::eval_all(deserialized_key1, ys1, pi);
  }
};
