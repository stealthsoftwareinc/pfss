/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  DO_UP_TO_INCLUDING_GEN(8, 8, 0, 0);
  {
    pfss_status const s =
        pfss_parse_key(&key1, NULL, key_blob_size);
    if (s != PFSS_NULL_POINTER) {
      exit_status = TEST_EXIT_FAIL;
    }
  }
  goto cleanup;
}
