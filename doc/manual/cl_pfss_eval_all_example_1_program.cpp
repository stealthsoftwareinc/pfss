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
  constexpr uint32_t beta = 67890;
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

  // Allocate two grids.
  std::vector<uint32_t> ys1(domain_size);
  std::vector<uint32_t> ys2(domain_size);

  // Start 8 threads to evaluate the first grid. The first thread will
  // evaluate the domain elements whose bit patterns are 000x...x, the
  // second thread 001x...x, the third thread 010x...x, and so on.
  std::vector<std::future<void>> futures1;
  for (uint8_t i = 0; i != 8; ++i) {
    futures1.push_back(std::async(std::launch::async, [&key1, i, &ys1] {
      check_status(pfss_eval_all(key1.get(),
                                 &i,
                                 3, // 2^3 = 8
                                 PFSS_NATIVE_ENDIAN,
                                 (uint8_t *)ys1.data(),
                                 sizeof(ys1[0]),
                                 PFSS_NATIVE_ENDIAN));
    }));
  }

  // Start 16 threads to evaluate the second grid. The first thread will
  // evaluate the domain elements whose bit patterns are 0000x...x, the
  // second thread 0001x...x, the third thread 0010x...x, and so on.
  std::vector<std::future<void>> futures2;
  for (uint8_t i = 0; i != 16; ++i) {
    futures2.push_back(std::async(std::launch::async, [&key2, i, &ys2] {
      check_status(pfss_eval_all(key2.get(),
                                 &i,
                                 4, // 2^4 = 16
                                 PFSS_NATIVE_ENDIAN,
                                 (uint8_t *)ys2.data(),
                                 sizeof(ys2[0]),
                                 PFSS_NATIVE_ENDIAN));
    }));
  }

  // Wait for all threads to complete.
  for (std::future<void> & f : futures1) {
    f.get();
  }
  for (std::future<void> & f : futures2) {
    f.get();
  }

  // Collapse the two grids by summing all of their elements mod
  // 2^{range_bits} (2^32) to verify that we get beta (67890) as the
  // final result. Note that because the two keys came from the same
  // pfss_gen call, ys1[i] + ys2[i] should be zero mod 2^32 everywhere
  // except at alpha (12345), where it should be beta (67890).
  unsigned long ys1_sum = 0;
  unsigned long ys2_sum = 0;
  for (auto const y : ys1) {
    ys1_sum += (unsigned long)y; // naturally mod 2^n for some n >= 32
  }
  for (auto const y : ys2) {
    ys2_sum += (unsigned long)y; // naturally mod 2^n for some n >= 32
  }
  ys1_sum &= UINT32_MAX; // mod 2^32
  ys2_sum &= UINT32_MAX; // mod 2^32
  unsigned long const sum = (ys1_sum + ys2_sum) & UINT32_MAX;

  // Print out the results.
  std::cout << "ys1_sum = " << ys1_sum << "\n";
  std::cout << "ys2_sum = " << ys2_sum << "\n";
  std::cout << "sum = " << sum << "\n";
}
