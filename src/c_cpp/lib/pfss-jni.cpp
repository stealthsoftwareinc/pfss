//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <jni.h>
#include <limits>
#include <pfss.h>
#include <pfss/common.h>
#include <stdint.h>
#include <type_traits>

using namespace pfss;

namespace {

void validate_env(JNIEnv const * const env) {
  if (env == nullptr) {
    throw error(PFSS_NULL_POINTER);
  }
}

//
// Verifies that array has space for index + want elements.
//

template<class Array, class Index, class Want>
void validate_array(
    JNIEnv * const env,
    Array const array,
    Index const index,
    Want const want) {
  validate_env(env);
  if (array == nullptr) {
    throw error(PFSS_NULL_POINTER);
  }
  if (index < 0) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  if (want < 0) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  jint const length = env->GetArrayLength(array);
  using U1 = typename std::make_unsigned<Index>::type;
  using U2 = typename std::make_unsigned<Want>::type;
  using U3 = typename std::make_unsigned<jint>::type;
  using U = decltype(0U + (U1)0 + (U2)0 + (U3)0);
  auto const u_index = static_cast<U>(index);
  auto const u_want = static_cast<U>(want);
  auto const u_length = static_cast<U>(length);
  if (u_index > u_length || u_want > u_length - u_index) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
}

//
// Verifies that array has space for index + want1 * want2 elements.
//

template<class Array, class Index, class Want1, class Want2>
void validate_array(
    JNIEnv * const env,
    Array const array,
    Index const index,
    Want1 const want1,
    Want2 const want2) {
  if (want1 < 0) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  if (want2 < 0) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  using U1 = typename std::make_unsigned<Want1>::type;
  using U2 = typename std::make_unsigned<Want2>::type;
  using U3 = typename std::make_unsigned<jint>::type;
  using U = decltype(0U + (U1)0 + (U2)0 + (U3)0);
  auto const u_want1 = static_cast<U>(want1);
  auto const u_want2 = static_cast<U>(want2);
  auto const u_max = static_cast<U>(std::numeric_limits<jint>::max());
  if (u_want2 > 0 && u_want1 > u_max / u_want2) {
    throw error(PFSS_INVALID_ARGUMENT);
  }
  validate_array(env, array, index, u_want1 * u_want2);
}

template<class ValueType>
struct to_array_type {};
template<>
struct to_array_type<uint8_t> {
  using type = jbyteArray;
};
template<>
struct to_array_type<uint32_t> {
  using type = jintArray;
};

template<class ValueType>
class critical_array final {

  using array_type = typename to_array_type<ValueType>::type;

  JNIEnv * env;
  array_type array;
  ValueType * carray;

public:
  critical_array(JNIEnv * const env, array_type const array) :
      env(env),
      array(array) {
    carray = reinterpret_cast<ValueType *>(
        env->GetPrimitiveArrayCritical(array, nullptr));
    if (carray == nullptr) {
      throw error(PFSS_UNKNOWN_ERROR);
    }
  }

  ~critical_array() noexcept {
    if (env != nullptr) {
      env->ReleasePrimitiveArrayCritical(array, carray, 0);
    }
  }

  critical_array(critical_array && other) noexcept :
      env(other.env),
      array(other.array),
      carray(other.carray) {
    other.env = nullptr;
    other.array = nullptr;
    other.carray = nullptr;
  }

  critical_array(critical_array const &) = delete;
  critical_array & operator=(critical_array const &) = delete;
  critical_array & operator=(critical_array &&) = delete;

  template<class SizeType>
  ValueType & operator[](SizeType const i) noexcept {
    return carray[i];
  }

  template<class SizeType>
  ValueType const & operator[](SizeType const i) const noexcept {
    return carray[i];
  }
};

} // namespace

//----------------------------------------------------------------------
// pfss_gen_sizes
//----------------------------------------------------------------------

extern "C" JNIEXPORT jint JNICALL
Java_com_stealthsoftwareinc_pfss_pfss_pfss_1gen_1sizes(
    JNIEnv * const env,
    jclass,
    jint const domain_bits,
    jint const range_bits,
    jintArray const key_blob_size,
    jint const key_blob_size_index,
    jintArray const rand_buf_size,
    jint const rand_buf_size_index) {
  try {
    validate_env(env);
    validate_array(env, key_blob_size, key_blob_size_index, 1);
    validate_array(env, rand_buf_size, rand_buf_size_index, 1);
    critical_array<uint32_t> c_key_blob_size(env, key_blob_size);
    critical_array<uint32_t> c_rand_buf_size(env, rand_buf_size);
    pfss_status const status = pfss_gen_sizes(
        static_cast<uint32_t>(domain_bits),
        static_cast<uint32_t>(range_bits),
        &c_key_blob_size[key_blob_size_index],
        &c_rand_buf_size[rand_buf_size_index]);
    if (status == PFSS_OK) {
      if (c_key_blob_size[key_blob_size_index] >
          static_cast<uint32_t>(std::numeric_limits<jint>::max())) {
        throw error(PFSS_UNKNOWN_ERROR);
      }
      if (c_rand_buf_size[rand_buf_size_index] >
          static_cast<uint32_t>(std::numeric_limits<jint>::max())) {
        throw error(PFSS_UNKNOWN_ERROR);
      }
    }
    return status;
  } catch (error const & e) {
    return static_cast<jint>(e.status);
  } catch (...) {
    return static_cast<jint>(PFSS_UNKNOWN_ERROR);
  }
}

//----------------------------------------------------------------------
// pfss_gen
//----------------------------------------------------------------------

extern "C" JNIEXPORT jint JNICALL
Java_com_stealthsoftwareinc_pfss_pfss_pfss_1gen(
    JNIEnv * const env,
    jclass,
    jint const domain_bits,
    jint const range_bits,
    jbyteArray const alpha,
    jint const alpha_index,
    jint const alpha_size,
    jint const alpha_endianness,
    jbyteArray const beta,
    jint const beta_index,
    jint const beta_size,
    jint const beta_endianness,
    jbyteArray const key1_blob,
    jint const key1_blob_index,
    jbyteArray const key2_blob,
    jint const key2_blob_index,
    jbyteArray const rand_buf,
    jint const rand_buf_index) {
  try {
    uint32_t key_blob_size;
    uint32_t rand_buf_size;
    {
      pfss_status const status = pfss_gen_sizes(
          static_cast<uint32_t>(domain_bits),
          static_cast<uint32_t>(range_bits),
          &key_blob_size,
          &rand_buf_size);
      if (status != PFSS_OK) {
        return status;
      }
    }
    validate_env(env);
    validate_array(env, alpha, alpha_index, alpha_size);
    validate_array(env, beta, beta_index, beta_size);
    validate_array(env, key1_blob, key1_blob_index, key_blob_size);
    validate_array(env, key2_blob, key2_blob_index, key_blob_size);
    validate_array(env, rand_buf, rand_buf_index, rand_buf_size);
    critical_array<uint8_t> const c_alpha(env, alpha);
    critical_array<uint8_t> const c_beta(env, beta);
    critical_array<uint8_t> c_key1_blob(env, key1_blob);
    critical_array<uint8_t> c_key2_blob(env, key2_blob);
    critical_array<uint8_t> const c_rand_buf(env, rand_buf);
    return pfss_gen(
        static_cast<uint32_t>(domain_bits),
        static_cast<uint32_t>(range_bits),
        &c_alpha[alpha_index],
        static_cast<uint32_t>(alpha_size),
        static_cast<uint32_t>(alpha_endianness),
        &c_beta[beta_index],
        static_cast<uint32_t>(beta_size),
        static_cast<uint32_t>(beta_endianness),
        &c_key1_blob[key1_blob_index],
        &c_key2_blob[key2_blob_index],
        &c_rand_buf[rand_buf_index]);
  } catch (error const & e) {
    return static_cast<jint>(e.status);
  } catch (...) {
    return static_cast<jint>(PFSS_UNKNOWN_ERROR);
  }
}

//----------------------------------------------------------------------
// pfss_map_gen
//----------------------------------------------------------------------

extern "C" JNIEXPORT jint JNICALL
Java_com_stealthsoftwareinc_pfss_pfss_pfss_1map_1gen(
    JNIEnv * const env,
    jclass,
    jint const domain_bits,
    jint const range_bits,
    jbyteArray const alphas,
    jint const alphas_index,
    jint const alphas_count,
    jint const alpha_size,
    jint const alpha_endianness,
    jbyteArray const betas,
    jint const betas_index,
    jint const beta_size,
    jint const beta_endianness,
    jbyteArray const key1_blobs,
    jint const key1_blobs_index,
    jbyteArray const key2_blobs,
    jint const key2_blobs_index,
    jbyteArray const rand_bufs,
    jint const rand_bufs_index) {
  try {
    uint32_t key_blob_size;
    uint32_t rand_buf_size;
    {
      pfss_status const status = pfss_gen_sizes(
          static_cast<uint32_t>(domain_bits),
          static_cast<uint32_t>(range_bits),
          &key_blob_size,
          &rand_buf_size);
      if (status != PFSS_OK) {
        return status;
      }
    }
    validate_env(env);
    validate_array(env, alphas, alphas_index, alphas_count, alpha_size);
    validate_array(env, betas, betas_index, alphas_count, beta_size);
    validate_array(
        env, key1_blobs, key1_blobs_index, alphas_count, key_blob_size);
    validate_array(
        env, key2_blobs, key2_blobs_index, alphas_count, key_blob_size);
    validate_array(
        env, rand_bufs, rand_bufs_index, alphas_count, rand_buf_size);
    critical_array<uint8_t> const c_alphas(env, alphas);
    critical_array<uint8_t> const c_betas(env, betas);
    critical_array<uint8_t> c_key1_blobs(env, key1_blobs);
    critical_array<uint8_t> c_key2_blobs(env, key2_blobs);
    critical_array<uint8_t> const c_rand_bufs(env, rand_bufs);
    return pfss_map_gen(
        static_cast<uint32_t>(domain_bits),
        static_cast<uint32_t>(range_bits),
        &c_alphas[alphas_index],
        static_cast<uint32_t>(alphas_count),
        static_cast<uint32_t>(alpha_size),
        static_cast<uint32_t>(alpha_endianness),
        &c_betas[betas_index],
        static_cast<uint32_t>(beta_size),
        static_cast<uint32_t>(beta_endianness),
        &c_key1_blobs[key1_blobs_index],
        &c_key2_blobs[key2_blobs_index],
        &c_rand_bufs[rand_bufs_index]);
  } catch (error const & e) {
    return static_cast<jint>(e.status);
  } catch (...) {
    return static_cast<jint>(PFSS_UNKNOWN_ERROR);
  }
}

//----------------------------------------------------------------------
// pfss_reduce_sum
//----------------------------------------------------------------------

extern "C" JNIEXPORT jint JNICALL
Java_com_stealthsoftwareinc_pfss_pfss_pfss_1reduce_1sum(
    JNIEnv * const env,
    jclass,
    jint const range_bits,
    jbyteArray const ys,
    jint const ys_index,
    jint const ys_count,
    jint const y_size,
    jint const y_endianness,
    jbyteArray const z,
    jint const z_index,
    jint const z_size,
    jint const z_endianness) {
  try {
    validate_env(env);
    validate_array(env, ys, ys_index, ys_count, y_size);
    validate_array(env, z, z_index, z_size);
    critical_array<uint8_t> c_ys(env, ys);
    critical_array<uint8_t> c_z(env, z);
    return pfss_reduce_sum(
        range_bits,
        &c_ys[ys_index],
        ys_count,
        y_size,
        y_endianness,
        &c_z[z_index],
        z_size,
        z_endianness);
  } catch (error const & e) {
    return static_cast<jint>(e.status);
  } catch (...) {
    return static_cast<jint>(PFSS_UNKNOWN_ERROR);
  }
}
