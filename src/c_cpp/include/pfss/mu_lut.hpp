#include <cstdint>
namespace pfss {
// Lookup table for computing mu from Algorithm 3,
// line 3 in https://link.springer.com/content/pdf/10.1007/978-3-662-46803-6_12.pdf.
// Index this array by n + p - 1, where n is the # of domain bits and p is the number
// of parties.
constexpr std::uintmax_t mu_lut[127] = {0,
                                        2,
                                        2,
                                        3,
                                        4,
                                        6,
                                        8,
                                        12,
                                        16,
                                        23,
                                        32,
                                        46,
                                        64,
                                        91,
                                        128,
                                        182,
                                        256,
                                        363,
                                        512,
                                        725,
                                        1024,
                                        1449,
                                        2048,
                                        2897,
                                        4096,
                                        5793,
                                        8192,
                                        11586,
                                        16384,
                                        23171,
                                        32768,
                                        46341,
                                        65536,
                                        92682,
                                        131072,
                                        185364,
                                        262144,
                                        370728,
                                        524288,
                                        741456,
                                        1048576,
                                        1482911,
                                        2097152,
                                        2965821,
                                        4194304,
                                        5931642,
                                        8388608,
                                        11863284,
                                        16777216,
                                        23726567,
                                        33554432,
                                        47453133,
                                        67108864,
                                        94906266,
                                        134217728,
                                        189812532,
                                        268435456,
                                        379625063,
                                        536870912,
                                        759250125,
                                        1073741824,
                                        1518500250,
                                        2147483648,
                                        3037000500,
                                        4294967296,
                                        6074001000,
                                        8589934592,
                                        12148002000,
                                        17179869184,
                                        24296004000,
                                        34359738368,
                                        48592008000,
                                        68719476736,
                                        97184016000,
                                        137438953472,
                                        194368031999,
                                        274877906944,
                                        388736063997,
                                        549755813888,
                                        777472127994,
                                        1099511627776,
                                        1554944255988,
                                        2199023255552,
                                        3109888511976,
                                        4398046511104,
                                        6219777023951,
                                        8796093022208,
                                        12439554047902,
                                        17592186044416,
                                        24879108095804,
                                        35184372088832,
                                        49758216191608,
                                        70368744177664,
                                        99516432383216,
                                        140737488355328,
                                        199032864766431,
                                        281474976710656,
                                        398065729532861,
                                        562949953421312,
                                        796131459065722,
                                        1125899906842624,
                                        1592262918131444,
                                        2251799813685248,
                                        3184525836262887,
                                        4503599627370496,
                                        6369051672525773,
                                        9007199254740992,
                                        12738103345051546,
                                        18014398509481984,
                                        25476206690103092,
                                        36028797018963968,
                                        50952413380206184,
                                        72057594037927936,
                                        101904826760412368,
                                        144115188075855872,
                                        203809653520824736,
                                        288230376151711744,
                                        407619307041649472,
                                        576460752303423488,
                                        815238614083298944,
                                        1152921504606846976,
                                        1630477228166597888,
                                        2305843009213693952,
                                        3260954456333195776,
                                        4611686018427387904,
                                        6521908912666391552};

  template<class T>
  constexpr T compute_mu(int const n, int const p) noexcept {
    PFSS_STATIC_ASSERT(std::is_integral<T>::value);
    PFSS_CONSTEXPR_ASSERT(n > 0);
    PFSS_CONSTEXPR_ASSERT(p > 1);
    PFSS_CONSTEXPR_ASSERT(n <= 127); // These two together ensure we can
    PFSS_CONSTEXPR_ASSERT(p <= 127); // compute n+p-1 without overflowing.
    PFSS_CONSTEXPR_ASSERT(n + p - 1 >= 1);
    PFSS_CONSTEXPR_ASSERT(n + p - 1 <= 127);
    PFSS_CONSTEXPR_ASSERT(n + p - 1 < sizeof(mu_lut) / sizeof(*mu_lut));
    PFSS_CONSTEXPR_ASSERT(mu_lut[n + p - 1] <= type_max<T>::value);
    return T(mu_lut[n + p - 1]);
  }

  // Returns ceil(x / y).
  template<class T1, class T2>
  constexpr decltype(T1() / T2()) ceil_div(T1 const x,
                                           T2 const y) noexcept {
    PFSS_STATIC_ASSERT(std::is_integral<T1>::value);
    PFSS_STATIC_ASSERT(std::is_integral<T2>::value);
    PFSS_CONSTEXPR_ASSERT(x >= 0);
    PFSS_CONSTEXPR_ASSERT(y > 0);
    return x / y + (x % y != 0);
  }

  // Returns 2^64 - 1.
  constexpr unsigned long long mask64() noexcept {
    return get_mask<unsigned long long>(64);
  }

  // Returns ceil(2^n / mu).
  template<class T>
  constexpr unsigned long long compute_nu(int const n,
                                          T const mu) noexcept {
    PFSS_STATIC_ASSERT(std::is_integral<T>::value);
    PFSS_CONSTEXPR_ASSERT(n > 0);
    PFSS_CONSTEXPR_ASSERT(n <= 64);
    PFSS_CONSTEXPR_ASSERT(mu > 1);
    // Note: Given two integers x,y > 0, if x-1 is divisible by y, then
    // ceil(x/y) = (x-1)/y+1. If x nor x-1 is divisible by y, then
    // ceil(x/y) = ceil((x-1)/y).
    using ull = unsigned long long;
    return n < 64 ? ull(ceil_div(ull(1) << n, mu)) :
                    mu % 2 == 0 ?
                    ull(ceil_div(ull(1) << 63, mu / 2)) :
                    mask64() % mu == 0 ? ull(mask64() / mu + 1) :
                                         ull(ceil_div(mask64(), mu));
  }
} // namespace pfss
