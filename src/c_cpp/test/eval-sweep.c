/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>
/*
* tests gen and eval on 8-bit domain and ranges, sampling random domain 
* and range values d and r respectively. Specifically, this tests eval over 
* the entire 8-bit domain to check if eval(key1,x) + eval(key2,x) = 0 when 
* x != d, and r when x == d
*/
int main(void) {
  uint32_t db = 8;
  uint32_t rb = 8;

  // randomly sample a value from the domain
  uint32_t domain_max = (1ULL << db);
  uint32_t dv = ((uint32_t)rand()) % domain_max;

  // randomly sample a value from the range
  uint32_t range_max = (1ULL << rb);
  uint32_t rv = ((uint32_t)rand()) % range_max;
  
  DO_UP_TO_PARSE_KEY(db, rb, dv, rv);
  // run eval over the full domain
  for (uint32_t x = 0; x != domain_max; ++x) {
    uint32_t y1;
    uint32_t y2;
    {
      pfss_status const s = pfss_eval(
          key1,
          (uint8_t const *)&x,
          sizeof(x),
          PFSS_NATIVE_ENDIAN,
          (uint8_t *)&y1,
          sizeof(y1),
          PFSS_NATIVE_ENDIAN);
      if (s != PFSS_OK) {
        exit_status = TEST_EXIT_ERROR;
        goto cleanup;
      }
    }
    {
      pfss_status const s = pfss_eval(
          key2,
          (uint8_t const *)&x,
          sizeof(x),
          PFSS_NATIVE_ENDIAN,
          (uint8_t *)&y2,
          sizeof(y2),
          PFSS_NATIVE_ENDIAN);
      if (s != PFSS_OK) {
        exit_status = TEST_EXIT_ERROR;
        goto cleanup;
      }
    }
    uint32_t sum = (unsigned long)y1 + (unsigned long)y2;
    sum %= domain_max;
    if (x == dv) {
      if (sum != rv) {
        exit_status = TEST_EXIT_FAIL;
        printf(
          "Eval(k1, %lu) + Eval(k2, %lu) = "
          "%lu + %lu = %lu mod %lu (expected %lu)\n",
          (unsigned long)x,
          (unsigned long)x,
          (unsigned long)y1,
          (unsigned long)y2,
          (unsigned long)sum,
          (unsigned long)range_max,
          (unsigned long)rv);
      }
    } else {
      if (sum != 0) {
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }
  goto cleanup;
}
