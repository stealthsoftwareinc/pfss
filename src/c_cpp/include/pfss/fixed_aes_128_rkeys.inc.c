/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#include <pfss/generate_fixed_aes_rkeys.h>

int main(void) {
  unsigned char const key[] =
#include <pfss/fixed_aes_128_key.inc>
      ;
  return generate_fixed_aes_rkeys(key, sizeof(key));
}
