/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <TEST_EXIT.h>
#include <helpers.h>
#include <pfss.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int test1(void) {
  DO_UP_TO_PARSE_KEY(16, 32, 12345, 67890);
  {
    static uint32_t ys1[1UL << 16] = {0};
    static uint32_t ys2[1UL << 16] = {0};
    {
      uint8_t const xp = 0;
      {
        pfss_status const s = pfss_eval_all(
            key1,
            &xp,
            0,
            PFSS_NATIVE_ENDIAN,
            (uint8_t *)ys1,
            sizeof(*ys1),
            PFSS_NATIVE_ENDIAN);
        if (s != PFSS_OK) {
          fprintf(
              stderr,
              "pfss_eval_all failed: %s\n",
              pfss_get_status_name(s));
          exit_status = TEST_EXIT_ERROR;
          goto cleanup;
        }
      }
      {
        pfss_status const s = pfss_eval_all(
            key2,
            &xp,
            0,
            PFSS_NATIVE_ENDIAN,
            (uint8_t *)ys2,
            sizeof(*ys2),
            PFSS_NATIVE_ENDIAN);
        if (s != PFSS_OK) {
          fprintf(
              stderr,
              "pfss_eval_all failed: %s\n",
              pfss_get_status_name(s));
          exit_status = TEST_EXIT_ERROR;
          goto cleanup;
        }
      }
    }
    {
      uint32_t x;
      for (x = 0; x != (uint32_t)(1UL << 16); ++x) {
        {
          uint32_t y;
          pfss_status const s = pfss_eval(
              key1,
              (uint8_t const *)&x,
              sizeof(x),
              PFSS_NATIVE_ENDIAN,
              (uint8_t *)&y,
              sizeof(y),
              PFSS_NATIVE_ENDIAN);
          if (s != PFSS_OK) {
            fprintf(
                stderr,
                "pfss_eval failed: %s\n",
                pfss_get_status_name(s));
            exit_status = TEST_EXIT_ERROR;
            goto cleanup;
          }
          if (ys1[x] != y) {
            fprintf(stderr, "pfss_eval_all disagrees with pfss_eval\n");
            exit_status = TEST_EXIT_FAIL;
            goto cleanup;
          }
        }
        {
          uint32_t y;
          pfss_status const s = pfss_eval(
              key2,
              (uint8_t const *)&x,
              sizeof(x),
              PFSS_NATIVE_ENDIAN,
              (uint8_t *)&y,
              sizeof(y),
              PFSS_NATIVE_ENDIAN);
          if (s != PFSS_OK) {
            fprintf(
                stderr,
                "pfss_eval failed: %s\n",
                pfss_get_status_name(s));
            exit_status = TEST_EXIT_ERROR;
            goto cleanup;
          }
          if (ys2[x] != y) {
            fprintf(stderr, "pfss_eval_all disagrees with pfss_eval\n");
            exit_status = TEST_EXIT_FAIL;
            goto cleanup;
          }
        }
      }
      for (x = 0; x != (uint32_t)(1UL << 16); ++x) {
        uint32_t const sum =
            (uint32_t)((unsigned long)ys1[x] + (unsigned long)ys2[x]);
        if (x == domain_value) {
          if (sum != range_value) {
            fprintf(
                stderr,
                "pfss_eval_all (and pfss_eval) got a wrong answer\n");
            exit_status = TEST_EXIT_FAIL;
            goto cleanup;
          }
        } else {
          if (sum != 0) {
            fprintf(
                stderr,
                "pfss_eval_all (and pfss_eval) got a wrong answer\n");
            exit_status = TEST_EXIT_FAIL;
            goto cleanup;
          }
        }
      }
    }
  }
  goto cleanup;
}

int main(void) {
  int s = TEST_EXIT_PASS;
  s = (s != TEST_EXIT_PASS ? s : test1());
  return s;
}
