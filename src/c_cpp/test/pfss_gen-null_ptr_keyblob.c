/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  DO_UP_TO_NOT_INCLUDING_GEN(8,8,0,0);
  {
    uint32_t const x = 0;
    uint32_t const y = 0;
    pfss_status const status1 = pfss_gen(
        8,
        8,
        (uint8_t const *)&x,
        sizeof(x),
        PFSS_NATIVE_ENDIAN,
        (uint8_t const *)&y,
        sizeof(y),
        PFSS_NATIVE_ENDIAN,
        NULL,
        key2_blob,
        rand_buf);
    pfss_status const status2 = pfss_gen(
        8,
        8,
        (uint8_t const *)&x,
        sizeof(x),
        PFSS_NATIVE_ENDIAN,
        (uint8_t const *)&y,
        sizeof(y),
        PFSS_NATIVE_ENDIAN,
        key1_blob,
        NULL,
        rand_buf);
    if (status1 != PFSS_NULL_POINTER ||
        status2 != PFSS_NULL_POINTER) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  goto cleanup;
}
