/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  DO_UP_TO_PARSE_KEY(10,16,3,5);
  {
    uint32_t range_bits = 0;
    pfss_status const s = pfss_get_range_bits(key1, &range_bits);
    if (range_bits != 16) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  {
    uint32_t range_bits = 0;
    pfss_status const s = pfss_get_range_bits(key2, &range_bits);
    if (range_bits != 16) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  goto cleanup;
}
