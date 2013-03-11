/**
 * Horrible command-line parsing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#include "cmdopts.hpp"
#include <getopt.h>
#include <vector>
#include <functional>
#include <iostream>
#include <cstdlib>
#include <cassert>

using namespace std;

const CmdOpts *g_opts = 0;

void parse_args(int argc, char **argv)
{
  if(g_opts)
    return;

  vector<option> options;
  vector<const char *> help;
  vector<function<void(const char *)>> fns;

  auto add_opt = [&options, &fns](const char *name, int has_arg, function<void(const char *)> fn)
  {
    fns.push_back(fn);
    options.resize(options.size() + 1);
    option &opt = options.back();
    opt.name = name;
    opt.has_arg = has_arg;
    opt.flag = 0;
    opt.val = options.size() - 1;
  };

  CmdOpts *opts = new CmdOpts;

#define ADD_EN_DIS_FEATURE(feature, opt) \
  add_opt("enable-" feature, no_argument, [opts] \
      (const char *) { opts->opt = true;}); \
  add_opt("disable-" feature, no_argument, [opts] \
      (const char *) {opts->opt = false;})

  add_opt("help", no_argument, [&options, &help](const char *)
      {
        cerr << "Valid arguments:" << endl;
        for(auto &opt : options)
        {
          cerr << "  --" << opt.name << endl;
        }
        exit(0);
      });

  ADD_EN_DIS_FEATURE("anti-aliasing", aa);
  ADD_EN_DIS_FEATURE("bounding-volumes", bv);
  ADD_EN_DIS_FEATURE("aa-jitter", aa_jitter);
  ADD_EN_DIS_FEATURE("draw-bv", draw_bv);
  ADD_EN_DIS_FEATURE("draw-aa", draw_aa);

  add_opt("aa-grid", required_argument, [opts](const char *num)
      {
        const int n = strtol(num, 0, 0);
        if(n <= 0)
        {
          cerr << "Invalid positive integer argument passed to aa-grid" << endl;
          exit(1);
        }
        opts->aa_grid = n;
      });

  add_opt("threads", required_argument, [opts](const char *num)
      {
        const int n = strtol(num, 0, 0);
        if(n <= 0)
        {
          cerr << "Invalid positive integer argument passed to threads" << endl;
          exit(1);
        }
        opts->threads = n;
      });

  add_opt("aa-threshold", required_argument, [opts](const char *num)
      {
        const double n = strtof(num, 0);
        if(n <= 0)
        {
          cerr << "Invalid positive double argument passed to aa-threshold" << endl;
          exit(1);
        }
        opts->aa_threshold = n;
      });

  add_opt("aa-jitter", required_argument, [opts](const char *num)
      {
        const double n = strtof(num, 0);
        if(n <= 0)
        {
          cerr << "Invalid positive double argument passed to aa-jitter" << endl;
          exit(1);
        }
        opts->aa_jitter = n;
      });

  while(1)
  {
    int option_index = -1;
    const char c = getopt_long(argc, argv, "", options.data(), &option_index);
    if(c == -1)
      break;
    if(option_index == -1)
    {
      cerr << "Malformed command line arguments? Aborting..." << endl;
      abort();
    }
    else
      fns[option_index](optarg);
  }

  g_opts = opts;
}
