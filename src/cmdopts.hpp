/**
 * Horrible command-line parsing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#ifndef __CMDOPTS_HPP__
#define __CMDOPTS_HPP__

#include <cstring>

extern const struct CmdOpts
{
  CmdOpts()
  {
    aa = bv = true;
    aa_jitter = 0;
    aa_grid = 3;
    draw_bv = draw_aa = false;
    threads = 4;
    aa_threshold = 0.5;
    aa_jitter = 0.;
  }

  bool aa, bv;
  bool draw_bv;
  bool draw_aa;
  unsigned aa_grid;
  unsigned threads;
  double aa_threshold;
  double aa_jitter;
} *g_opts;

const CmdOpts defaults;

#define GETOPT(opt) (g_opts->opt)

void parse_args(int argc, char **argv);

#endif
