//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <nan.h>
#include <new>
#include <pfss.h>
#include <string>

namespace {

//
// The unwrap function family is used to ensure that any kind of Maybe
// object gets unwrapped.
//
// The reason for having a trivial overload that takes a v8::Local<T>
// and returns it unchanged is because the Nan::New function sometimes
// returns v8::Local<T> and sometimes returns v8::MaybeLocal<T>. Having
// this overload lets us always write unwrap(Nan::New(...)).
//

template<class T>
v8::Local<T> unwrap(v8::Local<T> const & x) {
  return x;
}

template<class T>
auto unwrap(T x) -> decltype(x.ToLocalChecked()) {
  return x.ToLocalChecked();
}

template<class T>
auto unwrap(T const & x) -> decltype(x.FromJust()) {
  return x.FromJust();
}

//
// The nan_new function family calls Nan::New and automatically unwraps
// its returned object via the unwrap function family. This gives us a
// simpler interface than writing unwrap(Nan::New(...)) repeatedly.
//

template<class T>
auto nan_new(T const & x) -> decltype(unwrap(Nan::New(x))) {
  return unwrap(Nan::New(x));
}

template<class R, class T>
auto nan_new(T const & x) -> decltype(unwrap(Nan::New<R>(x))) {
  return unwrap(Nan::New<R>(x));
}

// Nan::New doesn't seem to handle void * properly.
v8::Local<v8::External> nan_new(void * const x) {
  return unwrap(Nan::New<v8::External>(x));
}

// This lets us write nan_new(undefined()).
struct undefined {};
v8::Local<v8::Primitive> nan_new(undefined const &) {
  return Nan::Undefined();
}

//
// The pointed<T> class represents an array of T's that's being held in
// a block of memory that may or may not be suitably aligned for access
// via T *.
//

template<class T, bool Aligned = false>
class pointed final {
  unsigned char * const p;

public:
  // Determine whether p is suitably aligned for access via T *. The
  // idea is that you'd have code pathways for both pointed<T, false>
  // and pointed<T, true>, and then you'd call this function to decide
  // which one to use.
  static bool is_aligned(unsigned char const * const p) {
    void * q = const_cast<void *>(static_cast<void const *>(p));
    std::size_t n = sizeof(T);
    return std::align(alignof(T), n, q, n) != nullptr;
  }

  pointed(unsigned char * const p) : p(p) {
  }

  pointed(pointed const &) = default;
  pointed(pointed &&) = default;
  pointed & operator=(pointed const &) = default;
  pointed & operator=(pointed &&) = default;

  T operator=(T const x) {
    if (Aligned) {
      *reinterpret_cast<T *>(p) = x;
    } else {
      auto const q = reinterpret_cast<unsigned char const *>(&x);
      std::copy_n(q, sizeof(T), p);
    }
    return x;
  }

  operator T() const {
    if (Aligned) {
      return *reinterpret_cast<T const *>(p);
    } else {
      T x;
      auto const q = reinterpret_cast<unsigned char *>(&x);
      std::copy_n(p, sizeof(T), q);
      return x;
    }
  }

  template<class Index>
  pointed operator[](Index const i) {
    if (i < 0) {
      return pointed(p - static_cast<std::size_t>(-i) * sizeof(T));
    }
    return pointed(p + static_cast<std::size_t>(i) * sizeof(T));
  }

  template<class Index>
  pointed const operator[](Index const i) const {
    return (*const_cast<pointed *>(this))[i];
  }
};

//
// Ideally we'd use exceptions with a try block around all of the code
// in each of our entry points, but V8 likes -fno-exceptions. We could
// re-enable exceptions in binding.gyp with -fexceptions, but that's a
// bad idea. It's better to go along with what V8 likes.
//
// This means that we're forced to do a lot of repetitive typing, so we
// use macros to help us out. Note that none of the macro arguments are
// evaluated more than once except for the names, which are only used
// for identifiers.
//
// The idea for error cleanup is to use a lambda that returns bool for
// the main code instead of a try block.
//

// Declare a few things that every entry point needs.
#define COMMON() \
  bool js_return_is_set = false; \
  v8::Isolate * const isolate = info.GetIsolate(); \
  v8::Local<v8::Context> const context = isolate->GetCurrentContext()

// Set the JS return value to v and return s from the lambda. The reason
// for js_return_is_set is because this macro is also useful for smaller
// lambdas in the error cleanup code after the main lambda has returned,
// and we don't want to affect the JS return value from there.
#define RETURN(v, s) \
  do { \
    if (!js_return_is_set) { \
      info.GetReturnValue().Set(nan_new((v))); \
      js_return_is_set = true; \
    } \
    return (s); \
  } while (0)

// More readable shorthand for the two RETURN variants.
#define SUCCESS(v) RETURN((v), true)
#define FAILURE(v) RETURN((v), false)

// Verify that there are n arguments. If there aren't, set the JS return
// value to e and return false from the lambda.
#define ARG_COUNT(n, e) \
  do { \
    if (info.Length() != (n)) { \
      FAILURE((e)); \
    } \
  } while (0)

// Parse argument i as an object, naming it foo. If an error occurs, set
// the JS return value to e and return false from the lambda.
#define ARG_OBJECT(i, foo, e) \
  v8::Local<v8::Value> const & foo##_lv = info[(i)]; \
  if (!foo##_lv->IsObject()) { \
    FAILURE((e)); \
  } \
  v8::Local<v8::Object> const foo = unwrap(foo##_lv->ToObject(context))

// Set key k to value v in an object argument foo that has already been
// parsed with ARG_OBJECT. If an error occurs, set the JS return value
// appropriately and return false from the lambda.
#define OBJECT_SET(foo, k, v) \
  do { \
    if (!unwrap(foo->Set(context, nan_new((k)), nan_new((v))))) { \
      FAILURE(PFSS_UNKNOWN_ERROR); \
    } \
  } while (0)

// Parse argument i as a key, naming it foo. Also make the object
// available as foo_obj. If an error occurs, set the JS return value
// appropriately and return false from the lambda.
#define ARG_KEY(i, foo) \
  v8::Local<v8::Value> const & foo##_lv = info[(i)]; \
  if (!foo##_lv->IsObject()) { \
    FAILURE(PFSS_INVALID_ARGUMENT); \
  } \
  v8::Local<v8::Object> const foo##_obj = \
      unwrap(foo##_lv->ToObject(context)); \
  if (!unwrap(foo##_obj->Has(context, nan_new("ptr")))) { \
    FAILURE(PFSS_INVALID_ARGUMENT); \
  } \
  v8::Local<v8::Value> const & foo##_ptr = \
      unwrap(foo##_obj->Get(context, nan_new("ptr"))); \
  if (!foo##_ptr->IsExternal()) { \
    FAILURE(PFSS_INVALID_ARGUMENT); \
  } \
  pfss_key * const foo = \
      static_cast<pfss_key *>(unwrap(foo##_ptr->ToObject(context)) \
                                  .As<v8::External>() \
                                  ->Value())

// Parse argument i as a uint32_t, naming it foo. If an error occurs,
// set the JS return value to e and return false from the lambda.
#define ARG_UINT32(i, foo, e) \
  v8::Local<v8::Value> const & foo##_lv = info[(i)]; \
  if (!foo##_lv->IsUint32()) { \
    FAILURE((e)); \
  } \
  uint32_t const foo = unwrap(foo##_lv->Uint32Value(context))

#define NOT_CONST

// Parse argument i as a pointed<uint32_t>, naming it foo. If an error
// occurs, set the JS return value appropriately and return false from
// the lambda. V8 also knows the array size, so store that as foo_cap.
#define ARG_UINT32_ARRAY(i, foo, xconst) \
  int const foo##_ai = (i); \
  v8::Local<v8::Value> const & foo##_lv = info[foo##_ai]; \
  if (!foo##_lv->IsUint32Array()) { \
    FAILURE(PFSS_INVALID_ARGUMENT); \
  } \
  v8::Local<v8::Uint32Array> const foo##_la = \
      unwrap(foo##_lv->ToObject(context)).As<v8::Uint32Array>(); \
  std::shared_ptr<v8::BackingStore> const foo##_bs = \
      foo##_la->Buffer()->GetBackingStore(); \
  pointed<uint32_t> xconst foo( \
      static_cast<unsigned char *>(foo##_bs->Data()) + \
      foo##_la->ByteOffset()); \
  std::size_t const foo##_cap = foo##_la->Length(); \
  static_cast<void>(foo##_cap)

// Parse argument i as an unsigned char *, naming it foo. If an error
// occurs, set the JS return value appropriately and return false from
// the lambda. V8 also knows the array size, so store that as foo_cap.
#define ARG_UINT8_ARRAY(i, foo, xconst) \
  int const foo##_ai = (i); \
  v8::Local<v8::Value> const & foo##_lv = info[foo##_ai]; \
  if (!foo##_lv->IsUint8Array()) { \
    FAILURE(PFSS_INVALID_ARGUMENT); \
  } \
  v8::Local<v8::Uint8Array> const foo##_la = \
      unwrap(foo##_lv->ToObject(context)).As<v8::Uint8Array>(); \
  std::shared_ptr<v8::BackingStore> const foo##_bs = \
      foo##_la->Buffer()->GetBackingStore(); \
  unsigned char xconst * const foo = \
      static_cast<unsigned char *>(foo##_bs->Data()) + \
      foo##_la->ByteOffset(); \
  std::size_t const foo##_cap = foo##_la->Length(); \
  static_cast<void>(foo##_cap)

//
// Verify that an array argument x has enough space for n elements,
// where x has already been parsed with ARG_*_ARRAY and the starting
// index x_index is already known. If x doesn't have enough space, set
// the JS return value appropriately and return false from the lambda.
//
// Note that this macro is similar to the validate_array function in
// pfss-jni.cpp.
//

#define ARG_ARRAY_SIZE_1D(x, n) \
  do { \
    using U1 = std::make_unsigned<decltype(+x##_index)>::type; \
    using U2 = std::make_unsigned<decltype(n)>::type; \
    using U3 = std::make_unsigned<decltype(+x##_cap)>::type; \
    using U = decltype(0U + (U1)0 + (U2)0 + (U3)0); \
    auto const u_index = static_cast<U>(x##_index); \
    auto const u_want = static_cast<U>(n); \
    auto const u_length = static_cast<U>(x##_cap); \
    if (u_index > u_length || u_want > u_length - u_index) { \
      FAILURE(PFSS_INVALID_ARGUMENT); \
    } \
  } while (0)

//
// Verify that an array argument x has enough space for n1 * n2
// elements, where x has already been parsed with ARG_*_ARRAY and the
// starting index x_index is already known. If x doesn't have enough
// space, set the JS return value appropriately and return false from
// the lambda.
//
// Note that this macro is similar to the validate_array function in
// pfss-jni.cpp.
//

#define ARG_ARRAY_SIZE_2D(x, n1, n2) \
  do { \
    using U1 = std::make_unsigned<decltype(+x##_index)>::type; \
    using U2 = std::make_unsigned<decltype(n1)>::type; \
    using U3 = std::make_unsigned<decltype(n2)>::type; \
    using U4 = std::make_unsigned<decltype(+x##_cap)>::type; \
    using U = decltype(0U + (U1)0 + (U2)0 + (U3)0 + (U4)0); \
    auto const u_index = static_cast<U>(x##_index); \
    auto const u_want1 = static_cast<U>(n1); \
    auto const u_want2 = static_cast<U>(n2); \
    auto const u_length = static_cast<U>(x##_cap); \
    if (u_want2 > 0 && u_want1 > u_length / u_want2) { \
      FAILURE(PFSS_INVALID_ARGUMENT); \
    } \
    auto const u_want = u_want1 * u_want2; \
    if (u_index > u_length || u_want > u_length - u_index) { \
      FAILURE(PFSS_INVALID_ARGUMENT); \
    } \
  } while (0)

//
// The ARG_FUNCTION macro parses argument i as a function, naming it x.
// If an error occurs, the macro sets the JS return value appropriately
// and returns false from the enclosing lambda.
//

#define ARG_FUNCTION(i, x) \
  v8::Local<v8::Value> const & x##_lv = info[(i)]; \
  if (!x##_lv->IsFunction()) { \
    FAILURE(PFSS_INVALID_ARGUMENT); \
  } \
  v8::Local<v8::Function> const x = \
      unwrap(x##_lv->ToObject(context)).As<v8::Function>()

//----------------------------------------------------------------------

pfss_status call_async(
    std::function<pfss_status()> const & f,
    v8::Local<v8::Function> const & callback) {
  struct worker : Nan::AsyncWorker {
    std::function<pfss_status()> const f;
    pfss_status status = PFSS_UNKNOWN_ERROR;
    void Execute() override {
      status = f();
    }
    void handle(pfss_status const s) {
      Nan::HandleScope scope;
      static_cast<void>(scope);
      v8::Local<v8::Value> argv[] = {nan_new(s)};
      auto const argc = sizeof(argv) / sizeof(*argv);
      unwrap((*callback)(async_resource, argc, argv));
    }
    void HandleOKCallback() override {
      handle(status);
    }
    void HandleErrorCallback() override {
      handle(PFSS_UNKNOWN_ERROR);
    }
    worker(
        Nan::Callback * const callback,
        std::function<pfss_status()> const & f) :
        Nan::AsyncWorker(callback),
        f(f) {
    }
  };
  Nan::Callback * const c = new (std::nothrow) Nan::Callback(callback);
  if (c == nullptr) {
    return PFSS_UNKNOWN_ERROR;
  }
  worker * const w = new (std::nothrow) worker(c, f);
  if (w == nullptr) {
    delete c;
    return PFSS_UNKNOWN_ERROR;
  }
  Nan::AsyncQueueWorker(w); // will deallocate c and w
  return PFSS_OK;
}

//----------------------------------------------------------------------
// pfss_get_status_name
//----------------------------------------------------------------------

NAN_METHOD(pfss_get_status_name) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(1, "PFSS_UNKNOWN_STATUS");
    ARG_UINT32(0, status, "PFSS_UNKNOWN_STATUS");
    SUCCESS(::pfss_get_status_name(status));
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_gen_sizes
//----------------------------------------------------------------------

NAN_METHOD(pfss_gen_sizes) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(6, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(0, domain_bits, PFSS_INVALID_DOMAIN);
    ARG_UINT32(1, range_bits, PFSS_INVALID_RANGE);
    ARG_UINT32_ARRAY(2, key_blob_size, NOT_CONST);
    ARG_UINT32(3, key_blob_size_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32_ARRAY(4, rand_buf_size, NOT_CONST);
    ARG_UINT32(5, rand_buf_size_index, PFSS_INVALID_ARGUMENT);
    ARG_ARRAY_SIZE_1D(key_blob_size, 1);
    ARG_ARRAY_SIZE_1D(rand_buf_size, 1);
    uint32_t kbs;
    uint32_t rbs;
    ::pfss_status const status =
        ::pfss_gen_sizes(domain_bits, range_bits, &kbs, &rbs);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    key_blob_size[key_blob_size_index] = kbs;
    rand_buf_size[rand_buf_size_index] = rbs;
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_gen
//----------------------------------------------------------------------

NAN_METHOD(pfss_gen) {
  COMMON();
  uint32_t key_blob_size = 0;
  uint32_t rand_buf_size = 0;
  bool const success = [&]() {
    ARG_COUNT(16, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(0, domain_bits, PFSS_INVALID_DOMAIN);
    ARG_UINT32(1, range_bits, PFSS_INVALID_RANGE);
    ARG_UINT8_ARRAY(2, alpha, const);
    ARG_UINT32(3, alpha_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(4, alpha_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(5, alpha_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(6, beta, const);
    ARG_UINT32(7, beta_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(8, beta_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(9, beta_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(10, key1_blob, NOT_CONST);
    ARG_UINT32(11, key1_blob_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(12, key2_blob, NOT_CONST);
    ARG_UINT32(13, key2_blob_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(14, rand_buf, const);
    ARG_UINT32(15, rand_buf_index, PFSS_INVALID_ARGUMENT);
    {
      ::pfss_status const status = ::pfss_gen_sizes(
          domain_bits, range_bits, &key_blob_size, &rand_buf_size);
      if (status != PFSS_OK) {
        key_blob_size = 0;
        rand_buf_size = 0;
        FAILURE(status);
      }
    }
    ARG_ARRAY_SIZE_1D(alpha, alpha_size);
    ARG_ARRAY_SIZE_1D(beta, beta_size);
    ARG_ARRAY_SIZE_1D(key1_blob, key_blob_size);
    ARG_ARRAY_SIZE_1D(key2_blob, key_blob_size);
    ARG_ARRAY_SIZE_1D(rand_buf, rand_buf_size);
    {
      ::pfss_status const status = ::pfss_gen(
          domain_bits,
          range_bits,
          alpha + alpha_index,
          alpha_size,
          alpha_endianness,
          beta + beta_index,
          beta_size,
          beta_endianness,
          key1_blob + key1_blob_index,
          key2_blob + key2_blob_index,
          rand_buf + rand_buf_index);
      if (status != PFSS_OK) {
        FAILURE(status);
      }
      SUCCESS(status);
    }
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_parse_key
//----------------------------------------------------------------------

NAN_METHOD(pfss_parse_key) {
  pfss_key * key = nullptr;
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(4, PFSS_INVALID_ARGUMENT);
    ARG_OBJECT(0, p_key, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(1, key_blob, const);
    ARG_UINT32(2, key_blob_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(3, key_blob_size, PFSS_INVALID_ARGUMENT);
    ARG_ARRAY_SIZE_1D(key_blob, key_blob_size);
    ::pfss_status const status = ::pfss_parse_key(
        &key, key_blob + key_blob_index, key_blob_size);
    if (status != PFSS_OK) {
      key = nullptr;
      FAILURE(status);
    }
    OBJECT_SET(p_key, "ptr", static_cast<void *>(key));
    OBJECT_SET(
        p_key,
        "id",
        std::to_string(reinterpret_cast<std::uintptr_t>(key)));
    SUCCESS(status);
  }();
  if (!success) {
    if (key != nullptr) {
      ::pfss_destroy_key(key);
    }
  }
}

//----------------------------------------------------------------------
// pfss_destroy_key
//----------------------------------------------------------------------

NAN_METHOD(pfss_destroy_key) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(1, PFSS_INVALID_ARGUMENT);
    ARG_KEY(0, key);
    ::pfss_status const status = ::pfss_destroy_key(key);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    OBJECT_SET(key_obj, "ptr", undefined());
    OBJECT_SET(key_obj, "id", undefined());
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_eval
//----------------------------------------------------------------------

NAN_METHOD(pfss_eval) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(9, PFSS_INVALID_ARGUMENT);
    ARG_KEY(0, key);
    ARG_UINT8_ARRAY(1, x, const);
    ARG_UINT32(2, x_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(3, x_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(4, x_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(5, y, NOT_CONST);
    ARG_UINT32(6, y_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(7, y_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(8, y_endianness, PFSS_INVALID_ARGUMENT);
    ARG_ARRAY_SIZE_1D(x, x_size);
    ARG_ARRAY_SIZE_1D(y, y_size);
    ::pfss_status const status = ::pfss_eval(
        key,
        x + x_index,
        x_size,
        x_endianness,
        y + y_index,
        y_size,
        y_endianness);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_reduce_sum
//----------------------------------------------------------------------

NAN_METHOD(pfss_reduce_sum) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(10, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(0, range_bits, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(1, ys, const);
    ARG_UINT32(2, ys_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(3, ys_count, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(4, y_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(5, y_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(6, z, NOT_CONST);
    ARG_UINT32(7, z_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(8, z_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(9, z_endianness, PFSS_INVALID_ARGUMENT);
    ARG_ARRAY_SIZE_2D(ys, ys_count, y_size);
    ARG_ARRAY_SIZE_1D(z, z_size);
    ::pfss_status const status = ::pfss_reduce_sum(
        range_bits,
        ys + ys_index,
        ys_count,
        y_size,
        y_endianness,
        z + z_index,
        z_size,
        z_endianness);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_reduce_sum_async
//----------------------------------------------------------------------

/*// tag::node_tag_pfss_reduce_sum_async[]

[[node_sec_pfss_reduce_sum_async]]
== The `pfss_reduce_sum_async` function

[source,subs="quotes"]
----
_pfss_status_ pfss_reduce_sum_async(
  _Uint32_           range_bits,
  _Uint8Array const_ ys,
  _Uint32_           ys_index,
  _Uint32_           ys_count,
  _Uint32_           y_size,
  _pfss_endianness_  y_endianness,
  _Uint8Array_       z,
  _Uint32_           z_index,
  _Uint32_           z_size,
  _pfss_endianness_  z_endianness,
  _function_         callback
)
----

The `pfss_reduce_sum_async` function calls the
<<node_sec_pfss_reduce_sum,`pfss_reduce_sum`>> function asynchronously
with all of the parameters except `callback`.
After the asynchronous call completes, `callback(s)` is called, where
`s` is the status code returned by the asynchronous call.
The function returns `PFSS_OK` upon success, or another status code if
the asynchronous call could not be initiated.

.A simple sum
[example]
====
The following program uses the `pfss_reduce_sum_async` function to
compute stem:[(0 + 1 + \ldots + 99999)\ \textrm{mod}\ 2^{32}]:

[source,javascript]
----
Object.assign(global, require("bindings")("pfss"));

let ys = Uint32Array.from(Array(100000).keys());
let z = new Uint32Array(1);

let status1 = pfss_reduce_sum_async(
  ys.BYTES_PER_ELEMENT * 8, // 32
  new Uint8Array(ys.buffer),
  0,
  ys.length, // 100000
  ys.BYTES_PER_ELEMENT, // 4
  PFSS_NATIVE_ENDIAN,
  new Uint8Array(z.buffer),
  0,
  z.BYTES_PER_ELEMENT, // 4
  PFSS_NATIVE_ENDIAN,
  function(status2) {
    if (status2 == PFSS_OK) {
      console.log("z = " + z[0]);
    } else {
      console.log(pfss_get_status_name(status2));
    }
  }
);

if (status1 != PFSS_OK) {
  console.log(pfss_get_status_name(status1));
}
----

Output:

[source]
----
z = 704982704
----
====

*/// end::node_tag_pfss_reduce_sum_async[]

NAN_METHOD(pfss_reduce_sum_async) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(11, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(0, range_bits, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(1, ys, const);
    ARG_UINT32(2, ys_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(3, ys_count, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(4, y_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(5, y_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(6, z, NOT_CONST);
    ARG_UINT32(7, z_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(8, z_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(9, z_endianness, PFSS_INVALID_ARGUMENT);
    ARG_FUNCTION(10, callback);
    ARG_ARRAY_SIZE_2D(ys, ys_count, y_size);
    ARG_ARRAY_SIZE_1D(z, z_size);
    pfss_status const status = call_async(
        [=]() {
          static_cast<void>(ys_bs); // ensure ys lifetime
          static_cast<void>(z_bs); // ensure z lifetime
          return ::pfss_reduce_sum(
              range_bits,
              ys + ys_index,
              ys_count,
              y_size,
              y_endianness,
              z + z_index,
              z_size,
              z_endianness);
        },
        callback);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_map_eval_reduce_sum
//----------------------------------------------------------------------

NAN_METHOD(pfss_map_eval_reduce_sum) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(10, PFSS_INVALID_ARGUMENT);
    ARG_KEY(0, key);
    ARG_UINT8_ARRAY(1, xs, const);
    ARG_UINT32(2, xs_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(3, xs_count, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(4, x_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(5, x_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(6, y, NOT_CONST);
    ARG_UINT32(7, y_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(8, y_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(9, y_endianness, PFSS_INVALID_ARGUMENT);
    ARG_ARRAY_SIZE_2D(xs, xs_count, x_size);
    ARG_ARRAY_SIZE_1D(y, y_size);
    ::pfss_status const status = ::pfss_map_eval_reduce_sum(
        key,
        xs + xs_index,
        xs_count,
        x_size,
        x_endianness,
        y + y_index,
        y_size,
        y_endianness);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
// pfss_map_eval_reduce_sum_async
//----------------------------------------------------------------------

/*// tag::node_tag_pfss_map_eval_reduce_sum_async[]

[[node_sec_pfss_map_eval_reduce_sum_async]]
== The `pfss_map_eval_reduce_sum_async` function

[source,subs="quotes"]
----
_pfss_status_ pfss_map_eval_reduce_sum_async(
  _pfss_key_         key,
  _Uint8Array const_ xs,
  _Uint32_           xs_index,
  _Uint32_           xs_count,
  _Uint32_           x_size,
  _pfss_endianness_  x_endianness,
  _Uint8Array_       y,
  _Uint32_           y_index,
  _Uint32_           y_size,
  _pfss_endianness_  y_endianness,
  _function_         callback
)
----

The `pfss_map_eval_reduce_sum_async` function calls the
<<node_sec_pfss_map_eval_reduce_sum,`pfss_map_eval_reduce_sum`>>
function asynchronously with all of the parameters except `callback`.
After the asynchronous call completes, `callback(s)` is called, where
`s` is the status code returned by the asynchronous call.
The function returns `PFSS_OK` upon success, or another status code if
the asynchronous call could not be initiated.

*/// end::node_tag_pfss_map_eval_reduce_sum_async[]

NAN_METHOD(pfss_map_eval_reduce_sum_async) {
  COMMON();
  bool const success = [&]() {
    ARG_COUNT(11, PFSS_INVALID_ARGUMENT);
    ARG_KEY(0, key);
    ARG_UINT8_ARRAY(1, xs, const);
    ARG_UINT32(2, xs_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(3, xs_count, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(4, x_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(5, x_endianness, PFSS_INVALID_ARGUMENT);
    ARG_UINT8_ARRAY(6, y, NOT_CONST);
    ARG_UINT32(7, y_index, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(8, y_size, PFSS_INVALID_ARGUMENT);
    ARG_UINT32(9, y_endianness, PFSS_INVALID_ARGUMENT);
    ARG_FUNCTION(10, callback);
    ARG_ARRAY_SIZE_2D(xs, xs_count, x_size);
    ARG_ARRAY_SIZE_1D(y, y_size);
    pfss_status const status = call_async(
        [=]() {
          static_cast<void>(xs_bs); // ensure xs lifetime
          static_cast<void>(y_bs); // ensure y lifetime
          return ::pfss_map_eval_reduce_sum(
              key,
              xs + xs_index,
              xs_count,
              x_size,
              x_endianness,
              y + y_index,
              y_size,
              y_endianness);
        },
        callback);
    if (status != PFSS_OK) {
      FAILURE(status);
    }
    SUCCESS(status);
  }();
  if (!success) {
    // Nothing to clean up.
  }
}

//----------------------------------------------------------------------
//
//

template<class Exports, class Context, class Name, class Entity>
void export_entity(
    Exports const & exports,
    Context const & context,
    Name const & name,
    Entity const & entity) {
  if (!unwrap(exports->Set(context, name, entity))) {
    // Force a crash.
    v8::Nothing<int>().FromJust();
  }
}

template<class Exports, class Context, class Name, class Constant>
void export_constant(
    Exports const & exports,
    Context const & context,
    Name const & name,
    Constant const & constant) {
  export_entity(exports, context, nan_new(name), nan_new(constant));
}

template<class Exports, class Context, class Name, class Function>
void export_function(
    Exports const & exports,
    Context const & context,
    Name const & name,
    Function const & function) {
  export_entity(
      exports,
      context,
      nan_new(name),
      unwrap(nan_new<v8::FunctionTemplate>(function)->GetFunction(
          context)));
}

void init(v8::Local<v8::Object> const exports) {
  v8::Local<v8::Context> const context = exports->CreationContext();
#define EXPORT_CONSTANT(x) export_constant(exports, context, #x, x)
#define EXPORT_FUNCTION(x) export_function(exports, context, #x, x)

  EXPORT_CONSTANT(PFSS_OK);
  EXPORT_CONSTANT(PFSS_UNKNOWN_ERROR);
  EXPORT_CONSTANT(PFSS_INVALID_ARGUMENT);
  EXPORT_CONSTANT(PFSS_NULL_POINTER);
  EXPORT_CONSTANT(PFSS_INVALID_DOMAIN);
  EXPORT_CONSTANT(PFSS_INVALID_RANGE);
  EXPORT_CONSTANT(PFSS_UNSUPPORTED_DOMAIN_AND_RANGE);
  EXPORT_CONSTANT(PFSS_DOMAIN_OVERFLOW);
  EXPORT_CONSTANT(PFSS_RANGE_OVERFLOW);
  EXPORT_CONSTANT(PFSS_MALFORMED_KEY);

  EXPORT_CONSTANT(PFSS_NATIVE_ENDIAN);
  EXPORT_CONSTANT(PFSS_LITTLE_ENDIAN);
  EXPORT_CONSTANT(PFSS_BIG_ENDIAN);

  EXPORT_FUNCTION(pfss_get_status_name);
  EXPORT_FUNCTION(pfss_gen_sizes);
  EXPORT_FUNCTION(pfss_gen);
  EXPORT_FUNCTION(pfss_parse_key);
  EXPORT_FUNCTION(pfss_destroy_key);
  EXPORT_FUNCTION(pfss_eval);
  EXPORT_FUNCTION(pfss_reduce_sum);
  EXPORT_FUNCTION(pfss_reduce_sum_async);
  EXPORT_FUNCTION(pfss_map_eval_reduce_sum);
  EXPORT_FUNCTION(pfss_map_eval_reduce_sum_async);

#undef EXPORT_FUNCTION
#undef EXPORT_CONSTANT
}

} // namespace

NODE_MODULE(pfss, init)
