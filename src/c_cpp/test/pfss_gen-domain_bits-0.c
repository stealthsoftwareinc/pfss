/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  DO_UP_TO_NOT_INCLUDING_GEN(8, 8, 0, 0);
  {
    uint32_t const x = 0;
    uint32_t const y = 0;
    pfss_status const status = pfss_gen(
        0,
        8,
        (uint8_t const *)&x,
        sizeof(x),
        PFSS_NATIVE_ENDIAN,
        (uint8_t const *)&y,
        sizeof(y),
        PFSS_NATIVE_ENDIAN,
        key1_blob,
        key2_blob,
        rand_buf);
    if (status != PFSS_INVALID_DOMAIN) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  goto cleanup;
}
