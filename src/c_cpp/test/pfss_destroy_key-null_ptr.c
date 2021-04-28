/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  {
    pfss_status const s =
        pfss_destroy_key(NULL);
    if (s != PFSS_NULL_POINTER) {
      return TEST_EXIT_FAIL;
    }
  }
  return TEST_EXIT_PASS;
}
