/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  return TEST_EXIT_SKIP;
#if 0
  {
    pfss_status const s =
        pfss_eval_context_create(NULL);
    if (s != PFSS_INVALID_ARGUMENT) {
      return TEST_EXIT_FAIL;
    }
  }
  return TEST_EXIT_PASS;
#endif
}
