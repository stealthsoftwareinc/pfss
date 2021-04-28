/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

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

  // initialize arrays to contain the domain and range vals
  uint8_t xs[domain_max];
  for (uint32_t i = 0; i != domain_max; ++i) {
    xs[i] = i;
  }
  
  uint32_t y1;
  uint32_t y2;
  // run eval over the full domain for both keys and sum
  // their results
  pfss_map_eval_reduce_sum(
    key1,
    (uint8_t const *)xs,
    domain_max,
    sizeof(*xs),
    PFSS_NATIVE_ENDIAN,
    (uint8_t *)&y1,
    sizeof(y1),
    PFSS_NATIVE_ENDIAN);

  pfss_map_eval_reduce_sum(
    key2,
    (uint8_t const *)xs,
    domain_max,
    sizeof(*xs),
    PFSS_NATIVE_ENDIAN,
    (uint8_t *)&y2,
    sizeof(y2),
    PFSS_NATIVE_ENDIAN);

    uint32_t sum = (unsigned long)y1 + (unsigned long)y2;
    sum %= range_max;
    if (sum != rv) {
        exit_status = TEST_EXIT_FAIL;
        printf(
          "%lu + %lu = %lu mod %lu (expected %lu)\n",
          (unsigned long)y1,
          (unsigned long)y2,
          (unsigned long)sum,
          (unsigned long)range_max,
          (unsigned long)rv);
    }
  goto cleanup;
}
