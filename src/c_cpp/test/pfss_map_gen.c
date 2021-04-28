/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <helpers.h>

int main(void) {
  uint32_t db = 8;
  uint32_t rb = 8;
  uint32_t batch_size = 1000;

  // dv will be used in the macro argument but not for the unit test.
  uint32_t domain_max = (1ULL << db);
  uint32_t dv = 0;

  // rv will be used in macro argument but not for the unit test.
  uint32_t range_max = (1ULL << rb);
  uint32_t rv = 0;
  
  DO_UP_TO_NOT_INCLUDING_GEN(db, rb, dv, rv);

  // initialize arrays to contain the domain and range vals.
  // For this unit test beta_i == i, for every ith key1 and key2 blob
  uint8_t alphas[batch_size];
  uint8_t betas[batch_size];
  for (uint32_t i = 0; i != batch_size; ++i) {
    alphas[i] = ((uint32_t)rand()) % domain_max;
    betas[i] = i;
  }

  // initialize the containers for keys and rand_bufs
  // note that usage of these variables differs from other tests in that
  // these are containers for a batch of keys and rand_bufs
  key1_blob = malloc(key_blob_size*batch_size);
  key2_blob = malloc(key_blob_size*batch_size);
  rand_buf = malloc(rand_buf_size*batch_size);
  if (key1_blob == NULL || key2_blob == NULL || rand_buf == NULL) {
    exit_status = TEST_EXIT_ERROR;
    goto cleanup;
  }
  for (uint32_t i = 0; i != rand_buf_size*batch_size; ++i) { 
    rand_buf[i] = rand(); 
  } 
  {
    pfss_status const s = pfss_map_gen(
        db,
        rb,
        alphas,
        batch_size,
        sizeof(*alphas),
        PFSS_NATIVE_ENDIAN,
        betas,
        sizeof(*betas),
        PFSS_NATIVE_ENDIAN,
        key1_blob,
        key2_blob,
        rand_buf);
    if (s != PFSS_OK) {
      printf("pfss_map_gen failed: %s\n", pfss_get_status_name(s));
      exit_status = TEST_EXIT_ERROR;
      goto cleanup;
    }
  }

  for (uint32_t i = 0; i != batch_size; ++i) {
    {
      pfss_status const s =
          pfss_parse_key(&key1, key1_blob+i*key_blob_size, key_blob_size);
      if (s != PFSS_OK) {
        printf("pfss_parse_key failed: %s\n", pfss_get_status_name(s));
        key1 = NULL;
        exit_status = TEST_EXIT_ERROR;
        goto cleanup;
      }
    }
    {
      pfss_status const s =
          pfss_parse_key(&key2, key2_blob+i*key_blob_size, key_blob_size);
      if (s != PFSS_OK) {
        printf("pfss_parse_key failed: %s\n", pfss_get_status_name(s));
        key2 = NULL;
        exit_status = TEST_EXIT_ERROR;
        goto cleanup;
      }
    }
    uint32_t y1;
    uint32_t y2;

    // run eval over the full domain
    for (uint32_t x = 0; x != domain_max; ++x) {
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
      if (x == alphas[i]) {
        if (sum != betas[i]) {
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
  }
  goto cleanup;
}
