//
// Copyright (C) 2012-2022 Stealth Software Technologies, Inc.
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program. If not, see
// <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#ifndef PFSS_SST_CATALOG_RNG_SHELL_HPP
#define PFSS_SST_CATALOG_RNG_SHELL_HPP

#include <algorithm>
#include <pfss/sst/catalog/PFSS_SST_ASSERT.hpp>
#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/is_byte_pointer.hpp>
#include <pfss/sst/catalog/is_negative.hpp>
#include <pfss/sst/catalog/min.hpp>
#include <pfss/sst/catalog/moved_from.hpp>
#include <pfss/sst/catalog/size_max_v.hpp>
#include <pfss/sst/catalog/type_max.hpp>
#include <pfss/sst/catalog/uchar_max_v.hpp>
#include <pfss/sst/catalog/unsigned_gt.hpp>
#include <pfss/sst/catalog/unsigned_lt.hpp>
#include <utility>
#include <vector>

namespace pfss { namespace sst {

template<class Core>
class rng_shell final : public Core {
  using read_size_t = typename Core::read_size_t;

  static constexpr read_size_t read_size_max() {
    return ::pfss::sst::type_max<read_size_t>::value;
  }

  static constexpr std::size_t buffer_size_limit() {
    return (::pfss::sst::unsigned_lt(::pfss::sst::size_max_v, read_size_max()) ?
                ::pfss::sst::size_max_v :
                static_cast<std::size_t>(read_size_max()))
           / 8;
  }

  static constexpr read_size_t read_size_limit() {
    return read_size_max() / 8;
  }

  ::pfss::sst::moved_from moved_from_;
  std::vector<unsigned char> buffer_;
  std::size_t i_;

  void refill() {
    PFSS_SST_ASSERT((!moved_from_));
    if (i_ == buffer_.size()) {
      this->read(buffer_.data(),
                 static_cast<read_size_t>(buffer_.size()));
      i_ = 0;
    }
  }

  template<class OutputIt, class Size>
  OutputIt generate(OutputIt dst, Size n) {
    PFSS_SST_ASSERT((!moved_from_));
    PFSS_SST_ASSERT((!::pfss::sst::is_negative(n)));
    while (n > 0) {
      refill();
      std::size_t const r = buffer_.size() - i_;
      Size const k = ::pfss::sst::unsigned_lt(n, r) ? n : static_cast<Size>(r);
      dst = std::copy_n(buffer_.data() + i_, k, dst);
      i_ += static_cast<std::size_t>(k);
      n -= k;
    }
    return dst;
  }

public:
  using result_type = unsigned char;

  static constexpr unsigned char min() noexcept {
    return 0;
  }

  static constexpr unsigned char max() noexcept {
    return ::pfss::sst::uchar_max_v;
  }

  template<class... Args>
  explicit rng_shell(Args &&... args)
      : Core(std::forward<Args>(args)...),
        buffer_((PFSS_SST_ASSERT((this->buffer_size() > 0)),
                 ::pfss::sst::min(this->buffer_size(), buffer_size_limit()))),
        i_(buffer_.size()) {
  }

  rng_shell(rng_shell const &) = delete;

  rng_shell(rng_shell &&) noexcept = default;

  rng_shell & operator=(rng_shell const &) = delete;

  rng_shell & operator=(rng_shell &&) = delete;

  ~rng_shell() noexcept = default;

  unsigned char operator()() {
    PFSS_SST_ASSERT((!moved_from_));
    refill();
    return buffer_[i_++];
  }

  template<class OutputIt,
           class Size,
           ::pfss::sst::enable_if_t<!::pfss::sst::is_byte_pointer<OutputIt>::value> = 0>
  OutputIt operator()(OutputIt const dst, Size const n) {
    PFSS_SST_ASSERT((!moved_from_));
    PFSS_SST_ASSERT((!::pfss::sst::is_negative(n)));
    return generate(dst, n);
  }

  template<class OutputIt,
           class Size,
           ::pfss::sst::enable_if_t<::pfss::sst::is_byte_pointer<OutputIt>::value> = 0>
  OutputIt operator()(OutputIt const dst, Size n) {
    PFSS_SST_ASSERT((!moved_from_));
    PFSS_SST_ASSERT((dst != nullptr));
    PFSS_SST_ASSERT((!::pfss::sst::is_negative(n)));
    unsigned char * p = reinterpret_cast<unsigned char *>(dst);
    while (::pfss::sst::unsigned_gt(n, buffer_size_limit())) {
      Size const k = ::pfss::sst::unsigned_lt(n, read_size_limit()) ?
                         n :
                         static_cast<Size>(read_size_limit());
      this->read(p, static_cast<read_size_t>(k));
      p += k;
      n -= k;
    }
    return reinterpret_cast<OutputIt>(generate(p, n));
  }
};

}} // namespace pfss::sst

#endif // #ifndef PFSS_SST_CATALOG_RNG_SHELL_HPP
