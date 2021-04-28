//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

// Include first to test independence.
#include <pfss/standard_benchmark_runner.hpp>
// Include twice to test idempotence.
#include <pfss/standard_benchmark_runner.hpp>

#include <chrono>
#include <cstdlib>
#include <future>
#include <pfss/base.hpp>
#include <pfss/standard_benchmark.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace pfss {

bool standard_benchmark_runner::has_been_started() {
  return state_ != state::init;
}

void standard_benchmark_runner::start() {
  if (state_ != state::init) {
    throw std::logic_error("pfss::standard_benchmark_runner::start "
                           "called at the wrong time");
  }
  future_ = std::async(std::launch::async, [this] {
    std::stringstream cin;
    std::stringstream cout;
    std::stringstream cerr;
    std::string argv0 = "foo";
    argv0 += '\0';
    std::vector<char *> argv{&argv0[0], nullptr};
    int const s = pfss::standard_benchmark(
        pfss::checked_cast<int>(argv.size() - 1),
        argv.data(),
        cin,
        cout,
        cerr);
    if (s != EXIT_SUCCESS) {
      throw std::runtime_error("pfss::standard_benchmark failed");
    }
    result_ = cout.str();
  });
  state_ = state::running;
}

bool standard_benchmark_runner::result_is_ready() {
  if (state_ == state::init) {
    return false;
  }
  if (state_ != state::running) {
    return true;
  }
  auto const zero = std::chrono::seconds::zero();
  if (future_.wait_for(zero) == std::future_status::ready) {
    state_ = state::ready;
  }
  return state_ == state::ready;
}

std::string const & standard_benchmark_runner::get_result() {
  if (state_ == state::init) {
    throw std::logic_error("pfss::standard_benchmark_runner::get_"
                           "result called at the wrong time");
  }
  if (state_ != state::gotten) {
    future_.get();
    state_ = state::gotten;
  }
  return result_;
}

} // namespace pfss
