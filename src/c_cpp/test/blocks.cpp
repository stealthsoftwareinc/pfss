//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <TEST_EXIT.h>
#include <array>
#include <iostream>
#include <pfss/common.h>
#include <stdexcept>
#include <string>

namespace {

struct fail : std::runtime_error {
  explicit fail(std::string const & what_arg) :
      std::runtime_error(what_arg) {
  }
  explicit fail(char const * const what_arg) :
      std::runtime_error(what_arg) {
  }
};

struct error : std::runtime_error {
  explicit error(std::string const & what_arg) :
      std::runtime_error(what_arg) {
  }
  explicit error(char const * const what_arg) :
      std::runtime_error(what_arg) {
  }
};

using uchar = unsigned char;
constexpr uchar uchar_max = pfss::uchar_max::value;
constexpr uchar uchar_msb = pfss::uchar_msb::value;

template<class Block>
struct tester final {

  using mimic = std::array<uchar, Block::bytes>;

  std::string name;

  tester(std::string const & name) : name(name) {
  }

  void test() {
    mimic m1;
    mimic m2;
    Block b1;

    m1.fill(0);
    b1.parse(m1.cbegin());
    if (b1.get_msb() != 0) {
      throw fail(name + "::get_msb() failed on all zeros");
    }

    m1.fill(uchar_max);
    b1.parse(m1.cbegin());
    if (b1.get_msb() != 1) {
      throw fail(name + "::get_msb() failed on all ones");
    }

    m1.fill(0);
    b1.parse(m1.cbegin());
    m1[m1.size() - 1] ^= uchar_msb;
    b1 = b1.set_msb(1);
    b1.serialize(m2.begin());
    if (m1 != m2) {
      throw fail(name + "::set_msb(1) failed on all zeros");
    }

    m1.fill(uchar_max);
    b1.parse(m1.cbegin());
    m1[m1.size() - 1] ^= uchar_msb;
    b1 = b1.set_msb(0);
    b1.serialize(m2.begin());
    if (m1 != m2) {
      throw fail(name + "::set_msb(0) failed on all ones");
    }

    m1.fill(0);
    b1.parse(m1.cbegin());
    m1[m1.size() - 1] ^= uchar_msb;
    b1 = b1.flip_msb();
    b1.serialize(m2.begin());
    if (m1 != m2) {
      throw fail(name + "::flip_msb() failed on all zeros");
    }

    m1.fill(uchar_max);
    b1.parse(m1.cbegin());
    m1[m1.size() - 1] ^= uchar_msb;
    b1 = b1.flip_msb();
    b1.serialize(m2.begin());
    if (m1 != m2) {
      throw fail(name + "::flip_msb() failed on all ones");
    }

    std::cout << "pass: " << name << std::endl;
  }
};

// This lets us pass macro arguments with commas in them.
#define UNWRAP(...) __VA_ARGS__
std::string unwrap(char const * const s) {
  std::string x(s);
  return x.substr(1, x.size() - 2);
}

#define TEST(Block) tester<UNWRAP Block>(unwrap(#Block)).test()
#define SKIP(Block) \
  (std::cout << "skip: " << unwrap(#Block) << std::endl)

} // namespace

int main() {
  try {

    TEST((pfss::chunked_block<std::uint64_t, 2>));

#if PFSS_WITH_SSE2
    TEST((pfss::m128i_block));
#else
    SKIP((pfss::m128i_block));
#endif

#if PFSS_WITH_ARM_CRYPTO
    TEST((pfss::uint8x16_block));
#else
    SKIP((pfss::uint8x16_block));
#endif

  } catch (fail const & e) {
    try {
      std::cout << e.what() << std::endl;
    } catch (...) {
    }
    return TEST_EXIT_FAIL;
  } catch (std::exception const & e) {
    try {
      std::cout << e.what() << std::endl;
    } catch (...) {
    }
    return TEST_EXIT_ERROR;
  } catch (...) {
    return TEST_EXIT_ERROR;
  }
  return TEST_EXIT_PASS;
}
