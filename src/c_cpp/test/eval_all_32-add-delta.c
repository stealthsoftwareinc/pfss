/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <pfss.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
* tests gen and eval on 8-bit domain and ranges, sampling random domain 
* and range values d and r respectively. Specifically, this tests eval over 
* the entire 8-bit domain to check if eval(key1,x) + eval(key2,x) = 0 when 
* x != d, and r when x == d
*/
int main(void) {
  return TEST_EXIT_SKIP;
#if 0
  int exit_status = TEST_EXIT_PASS;
  uint32_t key_blob_size;
  uint32_t rand_buf_size;
  uint8_t * key1_blob = NULL;
  uint8_t * key2_blob = NULL;
  pfss_key * key1 = NULL;
  pfss_key * key2 = NULL;
  uint8_t * rand_buf = NULL;
  uint32_t domain_bits = 8;
  uint32_t range_bits = 8;

  pfss_eval_context * p_context = NULL;

  srand(1); // set a seed for generating random domain and range values

  // randomly sample a value from the domain
  uint32_t domain_max = (1ULL << domain_bits);
  uint32_t domain_val = ((uint32_t)rand()) % domain_max;

  // randomly sample a value from the range
  uint32_t range_max = (1ULL << range_bits);
  uint32_t range_val = ((uint32_t)rand()) % range_max;
  {
    pfss_status const s = pfss_gen_sizes(
        domain_bits,
        range_bits,
        &key_blob_size,
        &rand_buf_size);
    if (s != PFSS_OK) {
      exit_status = TEST_EXIT_ERROR;
      goto cleanup;
    }
  }
  key1_blob = malloc(key_blob_size);
  key2_blob = malloc(key_blob_size);
  rand_buf = malloc(rand_buf_size);
  {
    uint32_t i;
    for (i = 0; i != rand_buf_size; ++i) {
      rand_buf[i] = rand();
    }
  }
  {
    pfss_status const s = pfss_gen(
        domain_bits,
        range_bits,
        domain_val,
        range_val,
        key1_blob,
        key2_blob,
        rand_buf);
    if (s != PFSS_OK) {
      exit_status = TEST_EXIT_ERROR;
      goto cleanup;
    }
  }
  {
    pfss_status const s =
        pfss_parse_key(&key1, key1_blob, key_blob_size);
    if (s != PFSS_OK) {
      key1 = NULL;
      exit_status = TEST_EXIT_ERROR;
      goto cleanup;
    }
  }
  {
    pfss_status const s =
        pfss_parse_key(&key2, key2_blob, key_blob_size);
    if (s != PFSS_OK) {
      key2 = NULL;
      exit_status = TEST_EXIT_ERROR;
      goto cleanup;
    }
  }
  {
    pfss_status const s = pfss_eval_context_create(&p_context);
    if (s != PFSS_OK) {
      exit_status = TEST_EXIT_ERROR;
      goto cleanup;
    }
  }
  // run eval over the full domain
  {
    pfss_op op = pfss_add_delta; // choose update query operation
    uint32_t y1
        [domain_max]; // initialize array of zeros to be updated by the test query
    uint32_t y2
        [domain_max]; // initialize array of zeros to be updated by the test query
    uint32_t scalar_res;
    {
      pfss_status const s =
          pfss_eval_all_32(p_context, key1, y1, &scalar_res, op);
      if (s != PFSS_OK) {
        exit_status = TEST_EXIT_ERROR;
        goto cleanup;
      }
    }
    {
      pfss_status const s =
          pfss_eval_all_32(p_context, key2, y2, &scalar_res, op);
      if (s != PFSS_OK) {
        exit_status = TEST_EXIT_ERROR;
        goto cleanup;
      }
    }

    // check correctness of update query: the only spot with the secret range value should be the sum
    // of the y vectors at the secret domain slot
    for (uint32_t x = 0; x != domain_max; ++x) {
      uint32_t sum = (unsigned long)y1[x] + (unsigned long)y2[x];
      sum %= domain_max;
      if (x == domain_val) {
        if (sum != range_val) {
          exit_status = TEST_EXIT_FAIL;
        }
      } else {
        if (sum != 0) {
          exit_status = TEST_EXIT_FAIL;
        }
      }
    }
  }

cleanup:
  if (key2 != NULL) (void)pfss_destroy_key(key2);
  if (key1 != NULL) (void)pfss_destroy_key(key1);
  (void)pfss_eval_context_destroy(p_context);
  free(rand_buf);
  free(key2_blob);
  free(key1_blob);
  return exit_status;
#endif
}
