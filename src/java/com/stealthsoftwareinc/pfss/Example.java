/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

package com.stealthsoftwareinc.pfss;

import java.io.File;
import java.math.BigInteger;
import java.nio.file.Files;
import java.security.SecureRandom;
import java.util.Arrays;
import static com.stealthsoftwareinc.pfss.pfss.*;

public class Example {
  public static void main(final String... args) throws Exception {

    final String dir =
        (args.length >= 1) ? args[0] : System.getProperty("user.dir");
    System.load(dir + "/libnettle.so");
    System.load(dir + "/libpfss.so");

    final int domain_bits = 80;
    final int range_bits = 16;
    final byte[] alpha = new BigInteger("2").pow(70).toByteArray();
    final byte[] beta = new BigInteger("1").toByteArray();

    final int[] key_blob_size = new int[1];
    final int[] rand_buf_size = new int[1];
    {
      final int status = pfss_gen_sizes(
          domain_bits, range_bits, key_blob_size, 0, rand_buf_size, 0);
      if (status != PFSS_OK) {
        throw new RuntimeException();
      }
    }
    System.out.println("key_blob_size = " + key_blob_size[0]);
    System.out.println("rand_buf_size = " + rand_buf_size[0]);

    final byte[] key1_blob = new byte[key_blob_size[0]];
    final byte[] key2_blob = new byte[key_blob_size[0]];
    final byte[] rand_buf = new byte[rand_buf_size[0]];
    new SecureRandom().nextBytes(rand_buf);
    {
      final int status = pfss_gen(
          domain_bits,
          range_bits,
          alpha,
          0,
          alpha.length,
          PFSS_BIG_ENDIAN,
          beta,
          0,
          beta.length,
          PFSS_BIG_ENDIAN,
          key1_blob,
          0,
          key2_blob,
          0,
          rand_buf,
          0);
      if (status != PFSS_OK) {
        throw new RuntimeException();
      }
    }
    System.out.println("key1_blob = " + Arrays.toString(key1_blob));
    System.out.println("key2_blob = " + Arrays.toString(key2_blob));

    Files.write(new File("key1_blob").toPath(), key1_blob);
    Files.write(new File("key2_blob").toPath(), key2_blob);
  }
}
