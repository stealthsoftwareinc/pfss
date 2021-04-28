//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_STANDARD_BENCHMARK_HPP
#define PFSS_STANDARD_BENCHMARK_HPP

#include <istream>
#include <ostream>

namespace pfss {

int standard_benchmark(int const argc,
                       char ** const argv,
                       std::istream & cin,
                       std::ostream & cout,
                       std::ostream & cerr);

} // namespace pfss

#endif // PFSS_STANDARD_BENCHMARK_HPP
