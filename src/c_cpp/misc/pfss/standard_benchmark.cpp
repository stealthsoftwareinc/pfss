//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

// Include first to test independence.
#include <pfss/standard_benchmark.hpp>
// Include twice to test idempotence.
#include <pfss/standard_benchmark.hpp>

#include <cstdlib>
#include <exception>
#include <pfss/base.hpp>
#include <pfss/benchmark.hpp>
#include <stdexcept>
#include <vector>

namespace pfss {

namespace {

void go(char * const argv0,
        std::vector<std::string> args,
        std::istream & cin,
        std::ostream & cout,
        std::ostream & cerr) {
  std::vector<char *> argv{argv0};
  for (auto & arg : args) {
    arg += '\0';
    argv.push_back(&arg[0]);
  }
  argv.push_back(nullptr);
  cout << "\n";
  int const s =
      pfss::benchmark(pfss::checked_cast<int>(argv.size() - 1),
                      &argv[0],
                      cin,
                      cout,
                      cerr);
  if (s != EXIT_SUCCESS) {
    throw std::runtime_error("");
  }
}

} // namespace

int standard_benchmark(int const argc,
                       char ** const argv,
                       std::istream & cin,
                       std::ostream & cout,
                       std::ostream & cerr) {

  decltype(cout.exceptions()) old_cout_exceptions{};
  bool have_old_cout_exceptions = false;

  try {

    if (argc == 0) {
      throw std::runtime_error("");
    }

    old_cout_exceptions = cout.exceptions();
    have_old_cout_exceptions = true;
    cout.exceptions(cout.badbit | cout.failbit);

    enum class mode_t {
      normal,
      rrt_experiments_phone,
      rrt_experiments_desktop,
    };
    mode_t mode = mode_t::normal;

    for (int i = 1; i < argc; ++i) {
      if (argv[i] == std::string("--normal")) {
        mode = mode_t::normal;
      } else if (argv[i] == std::string("--rrt-experiments-phone")) {
        mode = mode_t::rrt_experiments_phone;
      } else if (argv[i] == std::string("--rrt-experiments-desktop")) {
        mode = mode_t::rrt_experiments_desktop;
      } else {
        throw std::runtime_error(std::string("invalid argument: ")
                                 + argv[i]);
      }
    }

    pfss::checked<unsigned long> experiment_max_repeats = 10;

    {
      std::vector<std::vector<std::string>> db_rb_list = {
          {"80", "16"},
          {"128", "8"},
          {"128", "16"},
          {"128", "32"},
      };
      for (auto const & db_rb : db_rb_list) {
        if (mode == mode_t::rrt_experiments_phone) {
          go(argv[0],
             {
                 "domain_bits=" + db_rb[0],
                 "range_bits=" + db_rb[1],
                 "routine=pfss_map_gen",
                 "alphas_count=2240",
                 "max_time_ms=10000",
             },
             cin,
             cout,
             cerr);
        }
        if (mode == mode_t::rrt_experiments_desktop) {
          go(argv[0],
             {
                 "domain_bits=" + db_rb[0],
                 "range_bits=" + db_rb[1],
                 "routine=pfss_parse_key",
                 "max_time_ms=10000",
             },
             cin,
             cout,
             cerr);
          go(argv[0],
             {
                 "domain_bits=" + db_rb[0],
                 "range_bits=" + db_rb[1],
                 "routine=pfss_map_eval_bucket",
                 "max_time_ms=99999999",
                 "key_count=2240",
                 "xs_count=5000",
                 "max_repeats="
                     + std::to_string(experiment_max_repeats.value()),
             },
             cin,
             cout,
             cerr);
        }
      }
      if (mode == mode_t::rrt_experiments_desktop) {
        go(argv[0],
           {
               "domain_bits=128",
               "range_bits=16",
               "routine=pfss_map_eval_bucket",
               "max_time_ms=99999999",
               "key_count=2240",
               "xs_count=10000",
               "max_repeats="
                   + std::to_string(experiment_max_repeats.value()),
           },
           cin,
           cout,
           cerr);
        go(argv[0],
           {
               "domain_bits=128",
               "range_bits=16",
               "routine=pfss_map_eval_bucket",
               "max_time_ms=99999999",
               "key_count=2240",
               "xs_count=50000",
               "max_repeats="
                   + std::to_string(experiment_max_repeats.value()),
           },
           cin,
           cout,
           cerr);
      }
    }

    {
      std::vector<std::vector<std::string>> db_rb_list = {
          {"16", "16"},
          {"20", "16"},
          {"24", "16"},
          {"20", "32"},
          {"20", "64"},
      };
      for (auto const & db_rb : db_rb_list) {
        if (mode == mode_t::rrt_experiments_phone) {
          go(argv[0],
             {
                 "domain_bits=" + db_rb[0],
                 "range_bits=" + db_rb[1],
                 "routine=pfss_map_gen",
                 "alphas_count=1",
                 "max_time_ms=10000",
             },
             cin,
             cout,
             cerr);
        }
        if (mode == mode_t::rrt_experiments_desktop) {
          go(argv[0],
             {
                 "domain_bits=" + db_rb[0],
                 "range_bits=" + db_rb[1],
                 "routine=pfss_eval_all",
                 "xp_bits=0",
                 "max_time_ms=99999999",
                 "max_repeats="
                     + std::to_string(experiment_max_repeats.value()),
             },
             cin,
             cout,
             cerr);
        }
      }
    }

    if (mode == mode_t::normal) {
      std::vector<std::string> const domain_bits_values{"8",
                                                        "16",
                                                        "20",
                                                        "24",
                                                        "32",
                                                        "64",
                                                        "80",
                                                        "128"};
      std::vector<std::string> const range_bits_values{"16",
                                                       "32",
                                                       "64"};
      std::vector<std::string> const routine_values{
          "pfss_eval",
          "pfss_gen",
          "pfss_map_eval",
          "pfss_map_eval_reduce_sum",
          "pfss_map_gen",
          "pfss_parse_key",
      };
      for (auto const & domain_bits : domain_bits_values) {
        for (auto const & range_bits : range_bits_values) {
          for (auto const & routine : routine_values) {
            go(argv[0],
               {
                   "domain_bits=" + domain_bits,
                   "range_bits=" + range_bits,
                   "routine=" + routine,
               },
               cin,
               cout,
               cerr);
          }
        }
      }
    }

    if (mode == mode_t::normal) {
      std::vector<std::string> const domain_bits_values{"8",
                                                        "16",
                                                        "20",
                                                        "24"};
      std::vector<std::string> const range_bits_values{"16",
                                                       "32",
                                                       "64"};
      std::vector<std::string> const routine_values{
          "pfss_eval_all",
      };
      std::vector<std::string> const xp_bits_values{"0"};
      for (auto const & domain_bits : domain_bits_values) {
        for (auto const & range_bits : range_bits_values) {
          for (auto const & routine : routine_values) {
            for (auto const & xp_bits : xp_bits_values) {
              go(argv[0],
                 {
                     "domain_bits=" + domain_bits,
                     "range_bits=" + range_bits,
                     "routine=" + routine,
                     "xp_bits=" + xp_bits,
                 },
                 cin,
                 cout,
                 cerr);
            }
          }
        }
      }
    }

    cout.flush();
    cout.exceptions(old_cout_exceptions);

  } catch (std::exception const & e) {
    try {
      cerr << e.what() << "\n";
    } catch (...) {
    }
    if (have_old_cout_exceptions) {
      try {
        cout.exceptions(old_cout_exceptions);
      } catch (...) {
      }
    }
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
