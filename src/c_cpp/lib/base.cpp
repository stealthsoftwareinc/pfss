//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

#include <chrono>
#include <mutex>
#include <pfss/base.hpp>

namespace pfss {

//----------------------------------------------------------------------
// pfss::mono_time
//----------------------------------------------------------------------

std::once_flag mono_time_flag;

decltype(std::chrono::steady_clock::now()) mono_time_t0;

//----------------------------------------------------------------------

} // namespace pfss
