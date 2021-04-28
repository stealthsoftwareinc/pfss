/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

package com.stealthsoftwareinc.pfss;

public final class pfss {

  private pfss() {
  }

  public static final int PFSS_OK = 0;
  public static final int PFSS_UNKNOWN_ERROR = 1;
  public static final int PFSS_INVALID_ARGUMENT = 2;
  public static final int PFSS_NULL_POINTER = 3;
  public static final int PFSS_INVALID_DOMAIN = 4;
  public static final int PFSS_INVALID_RANGE = 5;
  public static final int PFSS_UNSUPPORTED_DOMAIN_AND_RANGE = 6;
  public static final int PFSS_DOMAIN_OVERFLOW = 7;
  public static final int PFSS_RANGE_OVERFLOW = 8;
  public static final int PFSS_MALFORMED_KEY = 9;

  public static final int PFSS_NATIVE_ENDIAN = 0;
  public static final int PFSS_LITTLE_ENDIAN = 1;
  public static final int PFSS_BIG_ENDIAN = 2;

  public static native int pfss_gen_sizes(
      int domain_bits,
      int range_bits,
      int[] key_blob_size,
      int key_blob_size_index,
      int[] rand_buf_size,
      int rand_buf_size_index);

  public static native int pfss_gen(
      int domain_bits,
      int range_bits,
      byte[] alpha,
      int alpha_index,
      int alpha_size,
      int alpha_endianness,
      byte[] beta,
      int beta_index,
      int beta_size,
      int beta_endianness,
      byte[] key1_blob,
      int key1_blob_index,
      byte[] key2_blob,
      int key2_blob_index,
      byte[] rand_buf,
      int rand_buf_index);

  public static native int pfss_map_gen(
      int domain_bits,
      int range_bits,
      byte[] alphas,
      int alphas_index,
      int alphas_count,
      int alpha_size,
      int alpha_endianness,
      byte[] betas,
      int betas_index,
      int beta_size,
      int beta_endianness,
      byte[] key1_blobs,
      int key1_blobs_index,
      byte[] key2_blobs,
      int key2_blobs_index,
      byte[] rand_bufs,
      int rand_bufs_index);

  public static native int pfss_reduce_sum(
      int range_bits,
      byte[] ys,
      int ys_index,
      int ys_count,
      int y_size,
      int y_endianness,
      byte[] z,
      int z_index,
      int z_size,
      int z_endianness);
}
