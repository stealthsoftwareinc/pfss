/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

// Include first to test independence.
#include <pfss.h>
// Include twice to test idempotence.
#include <pfss.h>

#include <TEST_EXIT.h>
#include <stdint.h>

int main(void) {
  uint32_t key_blob_size;
  uint32_t rand_buf_size;
  pfss_status const status =
      pfss_gen_sizes(0, 8, &key_blob_size, &rand_buf_size);
  if (status != PFSS_INVALID_DOMAIN) {
    return TEST_EXIT_FAIL;
  }
  return TEST_EXIT_PASS;
}
