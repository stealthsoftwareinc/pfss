#include <TEST_EXIT.h>
#include <pfss.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DO_DECLARATIONS(db_, rb_, dv_, rv_) \
  int exit_status = TEST_EXIT_PASS; \
  uint32_t domain_bits = db_; \
  uint32_t range_bits = rb_; \
  uint32_t domain_value = dv_; \
  uint32_t range_value = rv_; \
  uint32_t key_blob_size; \
  uint32_t rand_buf_size; \
  uint8_t * key1_blob = NULL; \
  uint8_t * key2_blob = NULL; \
  uint8_t * rand_buf = NULL; \
  pfss_key * key1 = NULL; \
  pfss_key * key2 = NULL; \
  goto start; \
  cleanup: \
  if (key2 != NULL) { \
    pfss_destroy_key(key2); \
  } \
  if (key1 != NULL) { \
    pfss_destroy_key(key1); \
  } \
  free(rand_buf); \
  free(key2_blob); \
  free(key1_blob); \
  return exit_status; \
  start:

#define DO_UP_TO_GEN_SIZES(db_, rb_, dv_, rv_) \
  DO_DECLARATIONS(db_, rb_, dv_, rv_); \
  { \
    pfss_status const s = pfss_gen_sizes( \
        domain_bits, \
        range_bits, \
        &key_blob_size, \
        &rand_buf_size); \
    if (s != PFSS_OK) { \
      printf("pfss_gen_sizes failed: %s\n", pfss_get_status_name(s)); \
      exit_status = TEST_EXIT_ERROR; \
      goto cleanup; \
    } \
  }

#define DO_UP_TO_NOT_INCLUDING_GEN(db_, rb_, dv_, rv_) \
  DO_UP_TO_GEN_SIZES(db_, rb_, dv_, rv_); \
  key1_blob = malloc(key_blob_size); \
  key2_blob = malloc(key_blob_size); \
  rand_buf = malloc(rand_buf_size); \
  if (key1_blob == NULL || key2_blob == NULL || rand_buf == NULL) { \
    exit_status = TEST_EXIT_ERROR; \
    goto cleanup; \
  } \
  { \
    uint32_t i; \
    for (i = 0; i != rand_buf_size; ++i) { \
      rand_buf[i] = rand(); \
    } \
  } \

#define DO_UP_TO_INCLUDING_GEN(db_, rb_, dv_, rv_) \
  DO_UP_TO_NOT_INCLUDING_GEN(db_, rb_, dv_, rv_); \
  { \
    pfss_status const s = pfss_gen( \
        domain_bits, \
        range_bits, \
        (uint8_t const *)&domain_value, \
        sizeof(domain_value), \
        PFSS_NATIVE_ENDIAN, \
        (uint8_t const *)&range_value, \
        sizeof(range_value), \
        PFSS_NATIVE_ENDIAN, \
        key1_blob, \
        key2_blob, \
        rand_buf); \
    if (s != PFSS_OK) { \
      printf("pfss_gen failed: %s\n", pfss_get_status_name(s)); \
      exit_status = TEST_EXIT_ERROR; \
      goto cleanup; \
    } \
  }

#define DO_UP_TO_PARSE_KEY(db_, rb_, dv_, rv_) \
  DO_UP_TO_INCLUDING_GEN(db_, rb_, dv_, rv_); \
  { \
    pfss_status const s = \
        pfss_parse_key(&key1, key1_blob, key_blob_size); \
    if (s != PFSS_OK) { \
      printf("pfss_parse_key failed: %s\n", pfss_get_status_name(s)); \
      key1 = NULL; \
      exit_status = TEST_EXIT_ERROR; \
      goto cleanup; \
    } \
  } \
  { \
    pfss_status const s = \
        pfss_parse_key(&key2, key2_blob, key_blob_size); \
    if (s != PFSS_OK) { \
      printf("pfss_parse_key failed: %s\n", pfss_get_status_name(s)); \
      key2 = NULL; \
      exit_status = TEST_EXIT_ERROR; \
      goto cleanup; \
    } \
  } \
// ...
