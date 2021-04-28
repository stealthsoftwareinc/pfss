/*
 * For the copyright information for this file, please search up the
 * directory tree for the first COPYING file.
 */

#ifndef PFSS_JNI_H
#define PFSS_JNI_H

#include <jni.h>
#include <pfss.h>

PFSS_EXTERN_C JNIEXPORT jint JNICALL
Java_com_stealthsoftwareinc_pfss_pfss_gen_1sizes(
    JNIEnv * env,
    jclass clazz,
    jint domain_bits,
    jint range_bits,
    jintArray key_blob_size,
    jint key_blob_size_index,
    jintArray rand_buf_size,
    jint rand_buf_size_index);

PFSS_EXTERN_C JNIEXPORT jint JNICALL
Java_com_stealthsoftwareinc_pfss_pfss_pfss_1gen(
    JNIEnv * env,
    jclass clazz,
    jint domain_bits,
    jint range_bits,
    jbyteArray alpha,
    jint alpha_index,
    jint alpha_size,
    jint alpha_endianness,
    jbyteArray beta,
    jint beta_index,
    jint beta_size,
    jint beta_endianness,
    jbyteArray key1_blob,
    jint key1_blob_index,
    jbyteArray key2_blob,
    jint key2_blob_index,
    jbyteArray rand_buf,
    jint rand_buf_index);

#endif /* PFSS_JNI_H */
