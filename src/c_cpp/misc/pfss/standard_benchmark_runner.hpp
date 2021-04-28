//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#ifndef PFSS_STANDARD_BENCHMARK_RUNNER_HPP
#define PFSS_STANDARD_BENCHMARK_RUNNER_HPP

#include <future>
#include <string>

namespace pfss {

class standard_benchmark_runner {
  enum class state { init, running, ready, gotten };
  state state_ = state::init;
  std::future<void> future_;
  std::string result_;

public:
  bool has_been_started();
  void start();
  bool result_is_ready();
  std::string const & get_result();
};

} // namespace pfss

#endif // PFSS_STANDARD_BENCHMARK_RUNNER_HPP
