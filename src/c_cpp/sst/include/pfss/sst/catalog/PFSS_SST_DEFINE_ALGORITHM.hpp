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

#ifndef PFSS_SST_CATALOG_PFSS_SST_DEFINE_ALGORITHM_HPP
#define PFSS_SST_CATALOG_PFSS_SST_DEFINE_ALGORITHM_HPP

#include <type_traits>
#include <utility>

#include <pfss/sst/catalog/enable_if_t.hpp>
#include <pfss/sst/catalog/enable_t.hpp>
#include <pfss/sst/catalog/remove_cv_t.hpp>

//
// It would be possible to implement this so that the function always
// performs ADL, e.g., so that ::pfss::sst::to_hex would always prefer an ADL
// match via plain to_hex(...) over using to_hex_functor. However, this
// has concerning long term properties, as ADL could produce surprising
// matches and there would be no way to opt out.
//
// It seems more prudent to stick with the approach that ::pfss::sst::to_hex
// will always use to_hex_functor. A caller can then opt in to ADL by
// writing "using ::pfss::sst::to_hex" and calling plain "to_hex", similar to
// the usual std::swap idiom.
//
// We'll also prevent ADL from ever matching ::pfss::sst::to_hex itself, as one
// would never want it to be matched, and it would be prone to doing so
// since it's a forwarding function.
//

#define PFSS_SST_DEFINE_ALGORITHM_rv(functor, F)                            \
  functor<::pfss::sst::enable_t, F, PFSS_SST_Args...>()(                          \
      ::std::forward<PFSS_SST_Args>(PFSS_SST_args)...)

#define PFSS_SST_DEFINE_ALGORITHM(function, functor, X, F)                  \
                                                                       \
  template<class...>                                                   \
  class functor {};                                                    \
                                                                       \
  namespace PFSS_SST_DEFINE_ALGORITHM_no_adl_##function {                   \
                                                                       \
    /* This overload is used when the function returns void. */        \
    template<X, class... PFSS_SST_Args>                                     \
    auto function(PFSS_SST_Args &&... PFSS_SST_args) noexcept(                   \
        noexcept(PFSS_SST_DEFINE_ALGORITHM_rv(functor, F)))                 \
        ->::pfss::sst::enable_if_t<                                            \
            std::is_same<                                              \
                ::pfss::sst::remove_cv_t<                                      \
                    decltype(PFSS_SST_DEFINE_ALGORITHM_rv(functor, F))>,    \
                void>::value,                                          \
            decltype(PFSS_SST_DEFINE_ALGORITHM_rv(functor, F))> {           \
      PFSS_SST_DEFINE_ALGORITHM_rv(functor, F);                             \
    }                                                                  \
                                                                       \
    /* This overload is used when the function returns non-void. */    \
    template<X, class... PFSS_SST_Args>                                     \
    auto function(PFSS_SST_Args &&... PFSS_SST_args) noexcept(                   \
        noexcept(PFSS_SST_DEFINE_ALGORITHM_rv(functor, F)))                 \
        ->::pfss::sst::enable_if_t<                                            \
            !std::is_same<                                             \
                ::pfss::sst::remove_cv_t<                                      \
                    decltype(PFSS_SST_DEFINE_ALGORITHM_rv(functor, F))>,    \
                void>::value,                                          \
            decltype(PFSS_SST_DEFINE_ALGORITHM_rv(functor, F))> {           \
      return PFSS_SST_DEFINE_ALGORITHM_rv(functor, F);                      \
    }                                                                  \
                                                                       \
  } /* namespace PFSS_SST_DEFINE_ALGORITHM_no_adl_##function */             \
                                                                       \
  /* "using namespace" is what prevents ADL. */                        \
  using namespace PFSS_SST_DEFINE_ALGORITHM_no_adl_##function;

#endif // #ifndef PFSS_SST_CATALOG_PFSS_SST_DEFINE_ALGORITHM_HPP
