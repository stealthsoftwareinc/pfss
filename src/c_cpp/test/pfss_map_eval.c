/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>
/*
* Similar test to eval-sweep, but uses pfss_map_eval on an array 
* containing the full 8-bit domain instead of plain pfss_eval. Fills
* a corresponding array of range values with the results of the evals
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

  // initialize arrays to contain the domain and range vals
  uint8_t ys1[range_max];
  uint8_t ys2[range_max];
  uint8_t xs[domain_max];
  for (uint32_t i = 0; i != domain_max; ++i) {
    xs[i] = i;
  }

  // run map_eval over the full domain for both keys
  pfss_map_eval(
    key1,
    (uint8_t const *)xs,
    domain_max,
    sizeof(*xs),
    PFSS_NATIVE_ENDIAN,
    (uint8_t *)ys1,
    sizeof(*ys1),
    PFSS_NATIVE_ENDIAN);

  pfss_map_eval(
    key2,
    (uint8_t const *)xs,
    domain_max,
    sizeof(*xs),
    PFSS_NATIVE_ENDIAN,
    (uint8_t *)ys2,
    sizeof(*ys2),
    PFSS_NATIVE_ENDIAN);

  for (uint32_t x = 0; x != domain_max; ++x) {
    uint32_t sum = (unsigned long)ys1[x] + (unsigned long)ys2[x];
    sum %= domain_max;
    if (x == dv) {
      if (sum != rv) {
        exit_status = TEST_EXIT_FAIL;
        printf(
          "Eval(k1, %lu) + Eval(k2, %lu) = "
          "%lu + %lu = %lu mod %lu (expected %lu)\n",
          (unsigned long)xs[x],
          (unsigned long)xs[x],
          (unsigned long)ys1[x],
          (unsigned long)ys2[x],
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
