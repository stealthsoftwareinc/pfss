/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  DO_UP_TO_PARSE_KEY(10,16,3,5);
  
  uint32_t x = 0;
  uint32_t y;
  {
    pfss_status const s = pfss_eval(
        NULL,
        (uint8_t const *)&x,
        sizeof(x),
        PFSS_NATIVE_ENDIAN,
        (uint8_t *)&y,
        sizeof(y),
        PFSS_NATIVE_ENDIAN);
    if (s != PFSS_NULL_POINTER) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  goto cleanup;
}
