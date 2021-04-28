/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <pfss.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int exit_status = TEST_EXIT_PASS;
  uint32_t key_blob_size;
  uint32_t rand_buf_size;
  uint8_t * key1_blob = NULL;
  uint8_t * key2_blob = NULL;
  pfss_key * key1 = NULL;
  pfss_key * key2 = NULL;
  uint8_t * rand_buf = NULL;
  {
    pfss_status const s =
        pfss_gen_sizes(10, 16, &key_blob_size, &rand_buf_size);
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
    uint32_t const x = 3;
    uint32_t const y = 5;
    pfss_status const s = pfss_gen(
        10,
        16,
        (uint8_t const *)&x,
        sizeof(x),
        PFSS_NATIVE_ENDIAN,
        (uint8_t const *)&y,
        sizeof(y),
        PFSS_NATIVE_ENDIAN,
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
  for (uint32_t x = 0; x != 10; ++x) {
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
    sum %= 65536;
    printf(
        "f(%lu) = (%lu + %lu) %% 65536 = %lu\n",
        (unsigned long)x,
        (unsigned long)y1,
        (unsigned long)y2,
        (unsigned long)sum);
    if (x == 3) {
      if (sum != 5) {
        exit_status = TEST_EXIT_FAIL;
      }
    } else {
      if (sum != 0) {
        exit_status = TEST_EXIT_FAIL;
      }
    }
  }
cleanup:
  if (key2 != NULL)
    (void)pfss_destroy_key(key2);
  if (key1 != NULL)
    (void)pfss_destroy_key(key1);
  free(rand_buf);
  free(key2_blob);
  free(key1_blob);
  return exit_status;
}
