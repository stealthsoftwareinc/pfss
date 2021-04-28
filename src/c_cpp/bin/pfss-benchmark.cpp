//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <iostream>
#include <pfss/benchmark.hpp>

int main(int const argc, char ** const argv) {
  return pfss::benchmark(argc, argv, std::cin, std::cout, std::cerr);
}
