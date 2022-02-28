#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <pfss.h>
#include <random>
#include <stdexcept>

namespace {

void check_status(pfss_status const s) {
  if (s != PFSS_OK) {
    throw std::runtime_error(pfss_get_status_name(s));
  }
}

} // namespace

int main() {
  constexpr uint32_t domain_bits = 16;
  constexpr uint32_t range_bits = 32;
  constexpr uint16_t alpha = 12345;
  constexpr uint32_t beta = 1;
  constexpr std::size_t domain_size = (std::size_t)1 << domain_bits;

  // Find out how big our buffers need to be.
  uint32_t key_blob_size;
  uint32_t rand_buf_size;
  check_status(pfss_gen_sizes(domain_bits,
                              range_bits,
                              &key_blob_size,
                              &rand_buf_size));

  // Allocate our buffers.
  std::vector<uint8_t> key1_blob((std::size_t)key_blob_size);
  std::vector<uint8_t> key2_blob((std::size_t)key_blob_size);
  std::vector<uint8_t> rand_buf((std::size_t)rand_buf_size);

  // Fill rand_buf with random bytes. Note that std::random_device is
  // not necessarily cryptographically secure. In practice you should
  // use something with better guarantees.
  std::random_device random_device;
  for (uint8_t & r : rand_buf) {
    r = (uint8_t)random_device();
  }

  // Generate the two key blobs.
  check_status(pfss_gen(domain_bits,
                        range_bits,
                        (uint8_t const *)&alpha,
                        (uint32_t)sizeof(alpha),
                        PFSS_NATIVE_ENDIAN,
                        (uint8_t const *)&beta,
                        (uint32_t)sizeof(beta),
                        PFSS_NATIVE_ENDIAN,
                        key1_blob.data(),
                        key2_blob.data(),
                        rand_buf.data()));

  // Parse the first key blob.
  pfss_key * key1_ptr;
  check_status(
      pfss_parse_key(&key1_ptr, key1_blob.data(), key_blob_size));
  std::unique_ptr<pfss_key, void (*)(pfss_key *)> const key1(
      key1_ptr,
      [](pfss_key * const k) { pfss_destroy_key(k); });

  // Parse the second key blob.
  pfss_key * key2_ptr;
  check_status(
      pfss_parse_key(&key2_ptr, key2_blob.data(), key_blob_size));
  std::unique_ptr<pfss_key, void (*)(pfss_key *)> const key2(
      key2_ptr,
      [](pfss_key * const k) { pfss_destroy_key(k); });

  // Allocate one main grid initialized to all zeros everywhere except
  // at alpha (12345), where it is 67890.
  std::vector<uint32_t> ys(domain_size);
  ys[alpha] = 67890;

  // Use 7 threads to evaluate each key into an ephemeral grid (as if by
  // calling the pfss_eval_all function) and compute the dot product of
  // each ephemeral grid with the main grid into a result vector zs.
  std::vector<uint32_t> zs(2);
  std::vector<pfss_key const *> const keys = {key1_ptr, key2_ptr};
  check_status(pfss_eval_all_dot(keys.data(),
                                 keys.size(),
                                 (uint8_t const *)ys.data(),
                                 sizeof(ys[0]),
                                 PFSS_NATIVE_ENDIAN,
                                 (uint8_t *)zs.data(),
                                 sizeof(zs[0]),
                                 PFSS_NATIVE_ENDIAN,
                                 7));

  // Sum the two results mod 2^{range_bits} (2^32) to verify that we get
  // ys[alpha] (67890) as the final result. Note that because the two
  // keys came from the same pfss_gen call and beta = 1, the two keys
  // work together to "select" the element at index alpha in ys.
  unsigned long sum = 0;
  for (auto const z : zs) {
    sum += (unsigned long)z; // naturally mod 2^n for some n >= 32
  }
  sum &= UINT32_MAX; // mod 2^32

  // Print out the results.
  std::cout << "zs[0] = " << zs[0] << "\n";
  std::cout << "zs[1] = " << zs[1] << "\n";
  std::cout << "sum = " << sum << "\n";
}
