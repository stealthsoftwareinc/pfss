//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

// Include first to test independence.
#include <pfss/benchmark.hpp>
// Include twice to test idempotence.
#include <pfss/benchmark.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <memory>
#include <ostream>
#include <pfss.h>
#include <pfss/base.hpp>
#include <pfss/common.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pfss {

namespace {

thread_local char const * g_argv0;
thread_local std::istream * g_cin;
thread_local std::ostream * g_cout;
thread_local std::ostream * g_cerr;
thread_local std::ifstream * g_urandom;

void print_to_stderr(std::string const & message) noexcept {
  try {
    *g_cerr << g_argv0 << ": " << message << "\n";
  } catch (...) {
  }
}

struct fatal_t : std::runtime_error {
  using std::runtime_error::runtime_error;
  fatal_t() : std::runtime_error("") {
  }
};

void do_not_optimize_out(void * const p) noexcept {
  assert(p != nullptr);
  unsigned char volatile * const q =
      static_cast<unsigned char volatile *>(p);
  *q = *q;
}

//----------------------------------------------------------------------

template<class T>
class arithmetic_wrapper_t
    : public pfss::boxed<T, arithmetic_wrapper_t<T>> {
  using base_type = pfss::boxed<T, arithmetic_wrapper_t<T>>;

public:
  using base_type::base_type;

  static constexpr auto max = type_max<T>();

  arithmetic_wrapper_t & operator+=(T const & v) {
    this->value() += v;
    return *this;
  }

  bool operator==(arithmetic_wrapper_t const & other) const noexcept {
    return this->value() == other.value();
  }
};

//----------------------------------------------------------------------

struct nanoseconds_t
    : arithmetic_wrapper_t<std::chrono::nanoseconds::rep> {
  using arithmetic_wrapper_t::arithmetic_wrapper_t;
};

nanoseconds_t x_mono_time_ns() {
  static thread_local decltype(std::chrono::steady_clock::now()) t0;
  static thread_local bool first_call = true;
  if (first_call) {
    t0 = std::chrono::steady_clock::now();
    first_call = false;
  }
  return nanoseconds_t(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now() - t0)
          .count());
}

//----------------------------------------------------------------------

template<class T, class = void>
struct parser_t;

//----------------------------------------------------------------------

template<class T>
struct parser_t<T,
                typename std::enable_if<+std::is_integral<T>()>::type> {

  // Parses the RHS of a parameter=value command-line argument.
  static T parse(std::string const & rhs) {
    decltype(rhs.size()) i = 0;
    if (i == rhs.size()) {
      throw fatal_t();
    }
    bool const negative = (rhs[0] == '-');
    if (negative) {
      ++i;
      if (i == rhs.size()) {
        throw fatal_t();
      }
    }
    if (rhs[i] == '0') {
      ++i;
      if (i != rhs.size()) {
        throw fatal_t();
      }
      return 0;
    }
    if (negative && std::is_unsigned<T>()) {
      throw fatal_t();
    }
    pfss::checked<T> x = 0;
    for (; i != rhs.size(); ++i) {
      if (rhs[i] < '0' || rhs[i] > '9') {
        throw fatal_t();
      }
      auto const d = rhs[i] - '0';
      if (negative) {
        x = x * 10 - d;
      } else {
        x = x * 10 + d;
      }
    }
    return x.value();
  }

  // Converts a parsed RHS back into a string for display.
  static std::string unparse(T x) {
    bool const negative = pfss::is_negative(x);
    std::string s;
    do {
      s += '0' + static_cast<char>(negative ? -(x % 10) : x % 10);
    } while ((x /= 10) != 0);
    if (negative) {
      s += '-';
    }
    std::reverse(s.begin(), s.end());
    return s;
  }
};

//----------------------------------------------------------------------

template<>
struct parser_t<double> {
  // Parses the RHS of a parameter=value command-line argument.
  static double parse(std::string const & rhs) {
    try {
      return std::stod(rhs);
    } catch (std::invalid_argument const &) {
      throw fatal_t();
    } catch (std::out_of_range const &) {
      throw fatal_t();
    }
  }

  // Converts a parsed RHS back into a string for display.
  static std::string unparse(double const & x) {
    return std::to_string(x);
  }
};

//----------------------------------------------------------------------

template<class T, std::map<std::string, T> const * M>
struct map_parser_t {
  PFSS_STATIC_ASSERT(M != nullptr);

  // Parses the RHS of a parameter=value command-line argument.
  static T parse(std::string const & rhs) {
    auto const it = M->find(rhs);
    if (it == M->end()) {
      throw fatal_t();
    }
    return it->second;
  }

  // Converts a parsed RHS back into a string for display.
  static std::string unparse(T const & x) {
    for (auto const & kv : *M) {
      if (x == kv.second) {
        return kv.first;
      }
    }
    assert(false);
    throw fatal_t();
  }
};

//----------------------------------------------------------------------

// clang-format off
#define LIST { \
  F(pfss_eval), \
  F(pfss_eval_all), \
  F(pfss_gen), \
  F(pfss_map_eval), \
  F(pfss_map_eval_bucket), \
  F(pfss_map_eval_reduce_sum), \
  F(pfss_map_gen), \
  F(pfss_parse_key), \
}
// clang-format on

#define F(x) x
enum class routine_t LIST;
#undef F

#define F(x)                                                           \
  { #x, routine_t::x }
std::map<std::string, routine_t> string_to_routine_map LIST;
#undef F

#undef LIST

template<>
struct parser_t<routine_t>
    : map_parser_t<routine_t, &string_to_routine_map> {};

//----------------------------------------------------------------------

struct endianness_t : arithmetic_wrapper_t<pfss_endianness> {
  using arithmetic_wrapper_t::arithmetic_wrapper_t;
};

#define F(x)                                                           \
  { #x, endianness_t(x) }
std::map<std::string, endianness_t> const string_to_endianness_map{
    F(PFSS_NATIVE_ENDIAN),
    F(PFSS_LITTLE_ENDIAN),
    F(PFSS_BIG_ENDIAN),
};
#undef F

template<>
struct parser_t<endianness_t>
    : map_parser_t<endianness_t, &string_to_endianness_map> {};

//----------------------------------------------------------------------

template<class T>
class parameter_t {
  std::string const name_;
  T value_;
  bool have_value_;
  bool value_is_default_;
  int argi_ = -1;
  mutable bool already_printed_ = false;

public:
  parameter_t(std::string const & name)
      : name_(name),
        have_value_(false) {
  }

  parameter_t(std::string const & name, T const & default_value)
      : name_(name),
        value_(default_value),
        have_value_(true),
        value_is_default_(true) {
  }

  parameter_t & operator=(T const & value) {
    value_ = value;
    have_value_ = true;
    value_is_default_ = false;
    argi_ = -1;
    return *this;
  }

  void reject(std::string const & rhs) const {
    throw fatal_t("argv[" + std::to_string(argi_) + "]: invalid "
                  + name_ + " value: " + rhs);
  }

  void reject() const {
    assert(have_value_);
    reject(parser_t<T>::unparse(value_));
  }

  bool parse(int const argi,
             std::string const & lhs,
             std::string const & rhs) {
    assert(argi > 0);
    if (lhs != name_) {
      return false;
    }
    argi_ = argi;
    if (have_value_ && !value_is_default_) {
      print_to_stderr("argv[" + std::to_string(argi)
                      + "]: repeated parameter: " + name_);
    }
    try {
      *this = parser_t<T>::parse(rhs);
    } catch (fatal_t const &) {
      reject(rhs);
    }
    return true;
  }

  void print_once() const {
    assert(have_value_);
    if (already_printed_) {
      return;
    }
    *g_cout << name_ << "=" << parser_t<T>::unparse(value_) << "\n";
    already_printed_ = true;
  }

  T value() const {
    if (!have_value_) {
      throw fatal_t("missing parameter: " + name_);
    }
    print_once(); // Automatically print the parameters that get used.
    return value_;
  }
};

template<class Integer>
nanoseconds_t ms_to_ns(parameter_t<Integer> const & ms) {
  if (pfss::is_negative(ms.value())) {
    ms.reject();
  }
  try {
    pfss::checked<nanoseconds_t::value_type> ns(ms.value());
    ns *= 1000000;
    return nanoseconds_t(ns.value());
  } catch (pfss::checked_error const &) {
    ms.reject();
  }
  throw std::logic_error("unreachable");
}

using op_count_t = unsigned long long;

template<class Integer, class Callable>
double measure_throughput(nanoseconds_t const max_time_ns,
                          Integer const max_repeats,
                          Callable f) {
  pfss::checked<nanoseconds_t::value_type> total_time_ns = 0;
  pfss::checked<op_count_t> total_op_count = 0;
  Integer total_repeats = 0;
  nanoseconds_t const t0 = x_mono_time_ns();
  do {
    auto const pair = f();
    total_time_ns += pair.first.value();
    total_op_count += pair.second;
    if (max_repeats != 0) {
      ++total_repeats;
    }
  } while (x_mono_time_ns().value() - t0.value() < max_time_ns.value()
           && (max_repeats == 0 || total_repeats < max_repeats));
  return total_op_count.value()
         / (total_time_ns.value() / 1000000000.0);
}

template<class OutputIt>
void fill_with_random_bytes(OutputIt first, OutputIt const last) {
  try {
    auto random = std::istreambuf_iterator<char>(*g_urandom);
    while (first != last) {
      *first++ = *random++;
    }
  } catch (std::ios_base::failure const &) {
    throw fatal_t("error reading from /dev/urandom");
  }
}

void check_status(std::string const & func, pfss_status const status) {
  if (status != PFSS_OK) {
    throw fatal_t(func + " failed: " + pfss_get_status_name(status));
  }
}

using key_t = std::unique_ptr<pfss_key, void (*)(pfss_key *)>;

key_t parse_key(uint8_t const * key_blob, uint32_t key_blob_size) {
  key_t key(nullptr, [](pfss_key * const p) { pfss_destroy_key(p); });
  pfss_key * p;
  check_status("pfss_parse_key",
               pfss_parse_key(&p, key_blob, key_blob_size));
  key.reset(p);
  return key;
}

//----------------------------------------------------------------------

// Returns the value of an integer itself (which is a no-op) or of an
// integer wrapped in a parameter_t.
template<class T,
         typename std::enable_if<+std::is_integral<T>(), int>::type = 0>
T unwrap(T const x) noexcept {
  return x;
}
template<class T>
T unwrap(parameter_t<T> const & x) {
  return x.value();
}

// Validates an integer itself (indicating a programming error) or an
// integer wrapped in a parameter_t (indicating an invalid command-line
// parameter).
template<class T,
         typename std::enable_if<+std::is_integral<T>(), int>::type = 0>
void validate(T const, bool const b) {
  if (!b) {
    throw std::logic_error("internal error");
  }
}
template<class T>
void validate(parameter_t<T> const & x, bool const b) {
  if (!b) {
    x.reject();
  }
}

template<class T, class Integer1, class Integer2>
std::vector<T> make_vector(Integer1 const & count1,
                           Integer2 const & count2) {
  validate(count1, unwrap(count1) > 0);
  validate(count2, unwrap(count2) > 0);
  std::vector<T> vector;
  pfss::checked<decltype(vector.size())> total = unwrap(count1);
  total *= unwrap(count2);
  vector.resize(total.value());
  return vector;
}

template<class Integer1>
auto get_alpha_size(Integer1 const & domain_bits)
    -> decltype(pfss::bits_to_bytes(unwrap(domain_bits))) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  return pfss::bits_to_bytes(unwrap(domain_bits));
}

template<class Integer1, class Integer2>
std::vector<uint8_t> make_alphas(Integer1 const & domain_bits,
                                 Integer2 const & count) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto alphas =
      make_vector<uint8_t>(get_alpha_size(unwrap(domain_bits)),
                           unwrap(count));
  fill_with_random_bytes(alphas.begin(), alphas.end());
  return alphas;
}

template<class Integer1>
std::vector<uint8_t> make_alpha(Integer1 const & domain_bits) {
  return make_alphas(domain_bits, 1);
}

template<class Integer1>
auto get_beta_size(Integer1 const & range_bits)
    -> decltype(pfss::bits_to_bytes(unwrap(range_bits))) {
  validate(range_bits, unwrap(range_bits) > 0);
  return pfss::bits_to_bytes(unwrap(range_bits));
}

template<class Integer1, class Integer2>
std::vector<uint8_t> make_betas(Integer1 const & range_bits,
                                Integer2 const & count) {
  validate(range_bits, unwrap(range_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto betas = make_vector<uint8_t>(get_beta_size(unwrap(range_bits)),
                                    unwrap(count));
  fill_with_random_bytes(betas.begin(), betas.end());
  return betas;
}

template<class Integer1>
std::vector<uint8_t> make_beta(Integer1 const & range_bits) {
  return make_betas(range_bits, 1);
}

template<class Integer1, class Integer2>
uint32_t get_key_blob_size(Integer1 const & domain_bits,
                           Integer2 const & range_bits) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(range_bits, unwrap(range_bits) > 0);
  uint32_t key_blob_size;
  uint32_t rand_buf_size;
  check_status(
      "pfss_gen_sizes",
      pfss_gen_sizes(pfss::checked_cast<uint32_t>(unwrap(domain_bits)),
                     pfss::checked_cast<uint32_t>(unwrap(range_bits)),
                     &key_blob_size,
                     &rand_buf_size));
  return key_blob_size;
}

template<class Integer1, class Integer2>
uint32_t get_rand_buf_size(Integer1 const & domain_bits,
                           Integer2 const & range_bits) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(range_bits, unwrap(range_bits) > 0);
  uint32_t key_blob_size;
  uint32_t rand_buf_size;
  check_status(
      "pfss_gen_sizes",
      pfss_gen_sizes(pfss::checked_cast<uint32_t>(unwrap(domain_bits)),
                     pfss::checked_cast<uint32_t>(unwrap(range_bits)),
                     &key_blob_size,
                     &rand_buf_size));
  return rand_buf_size;
}

template<class Integer1, class Integer2, class Integer3>
std::vector<uint8_t> make_key_blobs(Integer1 const & domain_bits,
                                    Integer2 const & range_bits,
                                    Integer3 const & count) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(range_bits, unwrap(range_bits) > 0);
  validate(count, unwrap(count) > 0);
  return make_vector<uint8_t>(
      unwrap(count),
      get_key_blob_size(unwrap(domain_bits), unwrap(range_bits)));
}

template<class Integer1, class Integer2>
std::vector<uint8_t> make_key_blob(Integer1 const & domain_bits,
                                   Integer2 const & range_bits) {
  return make_key_blobs(domain_bits, range_bits, 1);
}

template<class Integer1, class Integer2, class Integer3>
std::vector<uint8_t> make_rand_bufs(Integer1 const & domain_bits,
                                    Integer2 const & range_bits,
                                    Integer3 const & count) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(range_bits, unwrap(range_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto rand_bufs = make_vector<uint8_t>(
      unwrap(count),
      get_rand_buf_size(unwrap(domain_bits), unwrap(range_bits)));
  fill_with_random_bytes(rand_bufs.begin(), rand_bufs.end());
  return rand_bufs;
}

template<class Integer1, class Integer2>
std::vector<uint8_t> make_rand_buf(Integer1 const & domain_bits,
                                   Integer2 const & range_bits) {
  return make_rand_bufs(domain_bits, range_bits, 1);
}

template<class Integer1, class Integer2, class Integer3>
std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
gen_key_blob_pairs(Integer1 const & domain_bits,
                   Integer2 const & range_bits,
                   Integer3 const & count) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(range_bits, unwrap(range_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto const alpha_size = get_alpha_size(unwrap(domain_bits));
  auto const alphas = make_alphas(unwrap(domain_bits), unwrap(count));
  auto const beta_size = get_beta_size(unwrap(range_bits));
  auto const betas = make_betas(unwrap(range_bits), unwrap(count));
  auto key1_blobs = make_key_blobs(unwrap(domain_bits),
                                   unwrap(range_bits),
                                   unwrap(count));
  auto key2_blobs = make_key_blobs(unwrap(domain_bits),
                                   unwrap(range_bits),
                                   unwrap(count));
  auto const rand_bufs = make_rand_bufs(unwrap(domain_bits),
                                        unwrap(range_bits),
                                        unwrap(count));
  check_status(
      "pfss_map_gen",
      pfss_map_gen(pfss::checked_cast<uint32_t>(unwrap(domain_bits)),
                   pfss::checked_cast<uint32_t>(unwrap(range_bits)),
                   &alphas[0],
                   pfss::checked_cast<uint32_t>(unwrap(count)),
                   pfss::checked_cast<uint32_t>(alpha_size),
                   PFSS_NATIVE_ENDIAN,
                   &betas[0],
                   pfss::checked_cast<uint32_t>(beta_size),
                   PFSS_NATIVE_ENDIAN,
                   &key1_blobs[0],
                   &key2_blobs[0],
                   &rand_bufs[0]));
  return std::make_pair(std::move(key1_blobs), std::move(key2_blobs));
}

template<class Integer1, class Integer2>
std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
gen_key_blob_pairs(Integer1 const & domain_bits,
                   Integer2 const & range_bits) {
  return gen_key_blob_pairs(domain_bits, range_bits, 1);
}

template<class Integer1, class Integer2, class Integer3>
std::vector<std::pair<key_t, key_t>>
make_key_pairs(Integer1 const & domain_bits,
               Integer2 const & range_bits,
               Integer3 const & count) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(range_bits, unwrap(range_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto const key_blob_size =
      get_key_blob_size(unwrap(domain_bits), unwrap(range_bits));
  auto key_blobs = gen_key_blob_pairs(unwrap(domain_bits),
                                      unwrap(range_bits),
                                      unwrap(count));
  auto & key1_blobs = key_blobs.first;
  auto & key2_blobs = key_blobs.second;
  std::vector<std::pair<key_t, key_t>> keys;
  for (decltype(key1_blobs.size()) i = 0;
       pfss::unsigned_ne(i, unwrap(count));
       ++i) {
    auto key1 =
        parse_key(&key1_blobs[i * key_blob_size], key_blob_size);
    auto key2 =
        parse_key(&key2_blobs[i * key_blob_size], key_blob_size);
    keys.emplace_back(std::move(key1), std::move(key2));
  }
  return keys;
}

template<class Integer1, class Integer2>
std::pair<key_t, key_t> make_key_pair(Integer1 const & domain_bits,
                                      Integer2 const & range_bits) {
  auto key_pairs = make_key_pairs(domain_bits, range_bits, 1);
  return std::move(key_pairs[0]);
}

template<class Integer1>
auto get_x_size(Integer1 const & domain_bits)
    -> decltype(pfss::bits_to_bytes(unwrap(domain_bits))) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  return pfss::bits_to_bytes(unwrap(domain_bits));
}

template<class Integer1, class Integer2>
std::vector<uint8_t> make_xs(Integer1 const & domain_bits,
                             Integer2 const & count) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto xs = make_vector<uint8_t>(get_x_size(unwrap(domain_bits)),
                                 unwrap(count));
  fill_with_random_bytes(xs.begin(), xs.end());
  return xs;
}

template<class Integer1>
std::vector<uint8_t> make_x(Integer1 const & domain_bits) {
  return make_xs(domain_bits, 1);
}

template<class Integer1>
auto get_y_size(Integer1 const & range_bits)
    -> decltype(pfss::bits_to_bytes(unwrap(range_bits))) {
  validate(range_bits, unwrap(range_bits) > 0);
  return pfss::bits_to_bytes(unwrap(range_bits));
}

template<class Integer1, class Integer2>
std::vector<uint8_t> make_ys(Integer1 const & range_bits,
                             Integer2 const & count) {
  validate(range_bits, unwrap(range_bits) > 0);
  validate(count, unwrap(count) > 0);
  auto ys = make_vector<uint8_t>(get_y_size(unwrap(range_bits)),
                                 unwrap(count));
  return ys;
}

template<class Integer1>
std::vector<uint8_t> make_y(Integer1 const & range_bits) {
  return make_ys(range_bits, 1);
}

template<class Integer1>
auto get_xp_size(Integer1 const & xp_bits)
    -> decltype(pfss::bits_to_bytes(unwrap(xp_bits))) {
  validate(xp_bits, !pfss::is_negative(unwrap(xp_bits)));
  auto const xp_size = pfss::bits_to_bytes(unwrap(xp_bits));
  return xp_size == 0 ? 1 : xp_size;
}

template<class Integer1>
std::vector<uint8_t> make_xp(Integer1 const & xp_bits) {
  validate(xp_bits, !pfss::is_negative(unwrap(xp_bits)));
  auto xp = make_vector<uint8_t>(get_xp_size(unwrap(xp_bits)), 1);
  fill_with_random_bytes(xp.begin(), xp.end());
  return xp;
}

template<class Integer1>
std::size_t get_xp_ys_count(Integer1 const & domain_bits) {
  validate(domain_bits, unwrap(domain_bits) > 0);
  pfss::checked<std::size_t> v(1);
  v <<= unwrap(domain_bits);
  return v.value();
}

//----------------------------------------------------------------------
// Help
//----------------------------------------------------------------------

int print_help(std::string const & argv0) {
  *g_cout << "Usage: " << argv0 << R"( [<parameter>=<value>]...

Runs a benchmark on the PFSS library. The parameters of the benchmark
will be printed to standard output along with the resulting throughput.
Use --help=<parameter>[=<value>] for detailed help. Try --help=routine
to start.

Parameters:

      routine    max_time_ms    domain_bits    range_bits
                 max_repeats

      x_endianness    alpha_endianness    xp_endianness
      y_endianness    beta_endianness

alphas_count
xp_bits
xs_count

)" << std::flush;
  return EXIT_SUCCESS;
}

int print_help(std::string const & argv0, std::string const & topic) {
  if (false) {
  } else if (topic == "max_repeats") {
    *g_cout << argv0 << R"( max_repeats=<value>

Specifies the maximum number of repetitions for the benchmark.

The benchmark runs repeatedly until at least max_time_ms milliseconds
have elapsed or max_repeats repetitions have completed. If max_repeats
is zero, the number of repetitions is unlimited.
)" << std::flush;
  } else if (topic == "routine") {
    *g_cout << argv0 << R"( routine=<value>

Specifies which benchmark routine to run.

Values:
      pfss_eval
      pfss_eval_all
      pfss_gen
      pfss_map_eval
      pfss_map_eval_reduce_sum
      pfss_map_gen
      pfss_parse_key
)" << std::flush;
  } else {
    *g_cerr << argv0 << ": unknown help topic: " << topic << "\n"
            << std::flush;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------

int benchmark(int const argc,
              char ** const argv,
              std::istream & cin,
              std::ostream & cout,
              std::ostream & cerr) {

  g_cin = &cin;

  g_cout = &cout;
  decltype(cout.exceptions()) old_cout_exceptions{};
  bool have_old_cout_exceptions = false;

  g_cerr = &cerr;

  std::ifstream urandom;

  try {

    if (argc == 0) {
      throw std::runtime_error("");
    }
    g_argv0 = argv[0];

    old_cout_exceptions = cout.exceptions();
    have_old_cout_exceptions = true;
    cout.exceptions(cout.badbit | cout.failbit);

    urandom.exceptions(urandom.badbit | urandom.failbit);
    urandom.open("/dev/urandom", std::ios::in | std::ios::binary);
    g_urandom = &urandom;

    for (int argi = 1; argi < argc; ++argi) {
      std::string const arg = argv[argi];
      if (arg == "--help" || arg == "--hel" || arg == "--he"
          || arg == "--h") {
        return print_help(argv[0]);
      }
      if (arg.rfind("--help=", 0) == 0 || arg.rfind("--hel=", 0) == 0
          || arg.rfind("--he=", 0) == 0 || arg.rfind("--h=", 0) == 0) {
        return print_help(argv[0], arg.substr(arg.find("=") + 1));
      }
      if (arg.rfind("-", 0) == 0 && arg.rfind("--", 0) != 0) {
        auto const i = arg.find("h");
        if (i != arg.npos) {
          if (i == arg.size() - 1) {
            return print_help(argv[0]);
          }
          return print_help(argv[0], arg.substr(i + 1));
        }
      }
    }

    parameter_t<unsigned long> max_time_ms("max_time_ms", 5000);
    parameter_t<unsigned long> max_repeats("max_repeats", 0);
    parameter_t<endianness_t> alpha_endianness(
        "alpha_endianness",
        endianness_t(PFSS_NATIVE_ENDIAN));
    parameter_t<int> alphas_count("alphas_count", 100);
    parameter_t<endianness_t> beta_endianness(
        "beta_endianness",
        endianness_t(PFSS_NATIVE_ENDIAN));
    parameter_t<int> domain_bits("domain_bits");
    parameter_t<int> range_bits("range_bits");
    parameter_t<routine_t> routine("routine");
    parameter_t<unsigned long> key_count("key_count");
    parameter_t<endianness_t> x_endianness(
        "x_endianness",
        endianness_t(PFSS_NATIVE_ENDIAN));
    parameter_t<endianness_t> xp_endianness(
        "xp_endianness",
        endianness_t(PFSS_NATIVE_ENDIAN));
    parameter_t<int> xp_bits("xp_bits");
    parameter_t<int> xs_count("xs_count", 100);
    parameter_t<endianness_t> y_endianness(
        "y_endianness",
        endianness_t(PFSS_NATIVE_ENDIAN));

    for (int argi = 1; argi < argc; ++argi) {
      try {
        std::string const arg = argv[argi];
        auto const eq = arg.find('=');
        if (eq == arg.npos) {
          throw fatal_t("argument is not in parameter=value form: "
                        + arg);
        }
        std::string const lhs = arg.substr(0, eq);
        std::string const rhs = arg.substr(eq + 1);
        bool b = false;
        b = b || max_time_ms.parse(argi, lhs, rhs);
        b = b || max_repeats.parse(argi, lhs, rhs);
        b = b || alpha_endianness.parse(argi, lhs, rhs);
        b = b || alphas_count.parse(argi, lhs, rhs);
        b = b || beta_endianness.parse(argi, lhs, rhs);
        b = b || domain_bits.parse(argi, lhs, rhs);
        b = b || range_bits.parse(argi, lhs, rhs);
        b = b || routine.parse(argi, lhs, rhs);
        b = b || key_count.parse(argi, lhs, rhs);
        b = b || x_endianness.parse(argi, lhs, rhs);
        b = b || xp_bits.parse(argi, lhs, rhs);
        b = b || xp_endianness.parse(argi, lhs, rhs);
        b = b || xs_count.parse(argi, lhs, rhs);
        b = b || y_endianness.parse(argi, lhs, rhs);
        if (!b) {
          throw fatal_t("unknown parameter: " + lhs);
        }
      } catch (fatal_t const &) {
        throw;
      } catch (std::exception const & e) {
        throw fatal_t("argv[" + std::to_string(argi)
                      + "]: " + e.what());
      }
    }

    parameter_t<double> throughput("throughput");

    switch (routine.value()) {
      case routine_t::pfss_eval: {
        auto key = make_key_pair(domain_bits, range_bits).first;
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              auto const x_size = get_x_size(domain_bits);
              auto const y_size = get_y_size(range_bits);
              auto const x = make_x(domain_bits);
              auto y = make_y(range_bits);
              nanoseconds_t const t0 = x_mono_time_ns();
              pfss_status const status =
                  pfss_eval(key.get(),
                            &x[0],
                            pfss::checked_cast<uint32_t>(x_size),
                            x_endianness.value().value(),
                            &y[0],
                            pfss::checked_cast<uint32_t>(y_size),
                            y_endianness.value().value());
              nanoseconds_t const time_ns(x_mono_time_ns().value()
                                          - t0.value());
              check_status("pfss_eval", status);
              do_not_optimize_out(&y[0]);
              return std::make_pair(time_ns, 1);
            });
      } break;
      case routine_t::pfss_eval_all: {
        auto key = make_key_pair(domain_bits, range_bits).first;
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              auto const xp = make_xp(xp_bits);
              auto const ys_count = get_xp_ys_count(domain_bits);
              auto ys = make_ys(range_bits, ys_count);
              auto const y_size = get_y_size(range_bits);
              nanoseconds_t const t0 = x_mono_time_ns();
              pfss_status const status = pfss_eval_all(
                  key.get(),
                  &xp[0],
                  pfss::checked_cast<uint32_t>(xp_bits.value()),
                  xp_endianness.value().value(),
                  &ys[0],
                  pfss::checked_cast<uint32_t>(y_size),
                  y_endianness.value().value());
              nanoseconds_t const time_ns(x_mono_time_ns().value()
                                          - t0.value());
              check_status("pfss_eval_all", status);
              do_not_optimize_out(&ys[0]);
              return std::make_pair(time_ns,
                                    ys_count >> xp_bits.value());
            });
      } break;
      case routine_t::pfss_gen: {
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              auto const alpha = make_alpha(domain_bits);
              auto const beta = make_beta(range_bits);
              auto key1_blob = make_key_blob(domain_bits, range_bits);
              auto key2_blob = make_key_blob(domain_bits, range_bits);
              auto const rand_buf =
                  make_rand_buf(domain_bits, range_bits);
              nanoseconds_t const t0 = x_mono_time_ns();
              pfss_status const status = pfss_gen(
                  pfss::checked_cast<uint32_t>(domain_bits.value()),
                  pfss::checked_cast<uint32_t>(range_bits.value()),
                  &alpha[0],
                  pfss::checked_cast<uint32_t>(alpha.size()),
                  alpha_endianness.value().value(),
                  &beta[0],
                  pfss::checked_cast<uint32_t>(beta.size()),
                  beta_endianness.value().value(),
                  &key1_blob[0],
                  &key2_blob[0],
                  &rand_buf[0]);
              nanoseconds_t const time_ns(x_mono_time_ns().value()
                                          - t0.value());
              check_status("pfss_gen", status);
              do_not_optimize_out(&key1_blob[0]);
              do_not_optimize_out(&key2_blob[0]);
              return std::make_pair(time_ns, 1);
            });
      } break;
      case routine_t::pfss_map_eval: {
        auto key = make_key_pair(domain_bits, range_bits).first;
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              auto const x_size = get_x_size(domain_bits);
              auto const y_size = get_y_size(range_bits);
              auto const xs = make_xs(domain_bits, xs_count);
              auto ys = make_ys(range_bits, xs_count);
              nanoseconds_t const t0 = x_mono_time_ns();
              pfss_status const status = pfss_map_eval(
                  key.get(),
                  &xs[0],
                  pfss::checked_cast<uint32_t>(xs_count.value()),
                  pfss::checked_cast<uint32_t>(x_size),
                  x_endianness.value().value(),
                  &ys[0],
                  pfss::checked_cast<uint32_t>(y_size),
                  y_endianness.value().value());
              nanoseconds_t const time_ns(x_mono_time_ns().value()
                                          - t0.value());
              check_status("pfss_map_eval", status);
              do_not_optimize_out(&ys[0]);
              return std::make_pair(time_ns, xs_count.value());
            });
      } break;
      case routine_t::pfss_map_eval_bucket: {
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              pfss::checked<nanoseconds_t::value_type> time_ns = 0;
              pfss::checked<op_count_t> op_count = 0;
              for (decltype(+key_count.value()) i = 0;
                   i != key_count.value();
                   ++i) {
                auto key = make_key_pair(domain_bits, range_bits).first;
                auto const x_size = get_x_size(domain_bits);
                auto const y_size = get_y_size(range_bits);
                auto const xs = make_xs(domain_bits, xs_count);
                auto ys = make_ys(range_bits, xs_count);
                nanoseconds_t const t0 = x_mono_time_ns();
                pfss_status const status = pfss_map_eval(
                    key.get(),
                    &xs[0],
                    pfss::checked_cast<uint32_t>(xs_count.value()),
                    pfss::checked_cast<uint32_t>(x_size),
                    x_endianness.value().value(),
                    &ys[0],
                    pfss::checked_cast<uint32_t>(y_size),
                    y_endianness.value().value());
                time_ns += x_mono_time_ns().value() - t0.value();
                op_count += xs_count.value();
                check_status("pfss_map_eval", status);
                do_not_optimize_out(&ys[0]);
              }
              return std::make_pair(time_ns, op_count);
            });
      } break;
      case routine_t::pfss_map_eval_reduce_sum: {
        auto key = make_key_pair(domain_bits, range_bits).first;
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              auto const x_size = get_x_size(domain_bits);
              auto const y_size = get_y_size(range_bits);
              auto const xs = make_xs(domain_bits, xs_count);
              auto y = make_y(range_bits);
              nanoseconds_t const t0 = x_mono_time_ns();
              pfss_status const status = pfss_map_eval_reduce_sum(
                  key.get(),
                  &xs[0],
                  pfss::checked_cast<uint32_t>(xs_count.value()),
                  pfss::checked_cast<uint32_t>(x_size),
                  x_endianness.value().value(),
                  &y[0],
                  pfss::checked_cast<uint32_t>(y_size),
                  y_endianness.value().value());
              nanoseconds_t const time_ns(x_mono_time_ns().value()
                                          - t0.value());
              check_status("pfss_map_eval_reduce_sum", status);
              do_not_optimize_out(&y[0]);
              return std::make_pair(time_ns, xs_count.value());
            });
      } break;
      case routine_t::pfss_map_gen: {
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              auto const alpha_size = get_alpha_size(domain_bits);
              auto const alphas =
                  make_alphas(domain_bits, alphas_count);
              auto const beta_size = get_beta_size(domain_bits);
              auto const betas = make_betas(range_bits, alphas_count);
              auto key1_blobs =
                  make_key_blobs(domain_bits, range_bits, alphas_count);
              auto key2_blobs =
                  make_key_blobs(domain_bits, range_bits, alphas_count);
              auto const rand_bufs =
                  make_rand_bufs(domain_bits, range_bits, alphas_count);
              nanoseconds_t const t0 = x_mono_time_ns();
              pfss_status const status = pfss_map_gen(
                  pfss::checked_cast<uint32_t>(domain_bits.value()),
                  pfss::checked_cast<uint32_t>(range_bits.value()),
                  &alphas[0],
                  pfss::checked_cast<uint32_t>(alphas_count.value()),
                  pfss::checked_cast<uint32_t>(alpha_size),
                  alpha_endianness.value().value(),
                  &betas[0],
                  pfss::checked_cast<uint32_t>(beta_size),
                  beta_endianness.value().value(),
                  &key1_blobs[0],
                  &key2_blobs[0],
                  &rand_bufs[0]);
              nanoseconds_t const time_ns(x_mono_time_ns().value()
                                          - t0.value());
              check_status("pfss_map_gen", status);
              do_not_optimize_out(&key1_blobs[0]);
              do_not_optimize_out(&key2_blobs[0]);
              return std::make_pair(time_ns, alphas_count.value());
            });
      } break;
      case routine_t::pfss_parse_key: {
        throughput = measure_throughput(
            ms_to_ns(max_time_ms),
            unwrap(max_repeats),
            [&]() {
              pfss_key * key = nullptr;
              try {
                auto const key_blob_size =
                    get_key_blob_size(domain_bits, range_bits);
                auto const key_blobs =
                    gen_key_blob_pairs(domain_bits, range_bits);
                nanoseconds_t const t0 = x_mono_time_ns();
                pfss_status const status =
                    pfss_parse_key(&key,
                                   &key_blobs.first[0],
                                   key_blob_size);
                if (status != PFSS_OK) {
                  key = nullptr;
                }
                nanoseconds_t const time_ns(x_mono_time_ns().value()
                                            - t0.value());
                check_status("pfss_parse_key", status);
                do_not_optimize_out(key);
                pfss_destroy_key(key);
                key = nullptr;
                return std::make_pair(time_ns, 1);
              } catch (...) {
                pfss_destroy_key(key);
                throw;
              }
            });
      } break;
    }

    throughput.print_once();

    cout.flush();
    cout.exceptions(old_cout_exceptions);

  } catch (std::exception const & e) {
    if (have_old_cout_exceptions) {
      try {
        cout.exceptions(old_cout_exceptions);
      } catch (...) {
      }
    }
    print_to_stderr(e.what());
    return EXIT_FAILURE;
  } catch (...) {
    if (have_old_cout_exceptions) {
      try {
        cout.exceptions(old_cout_exceptions);
      } catch (...) {
      }
    }
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace pfss
