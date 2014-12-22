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
#include <sstream>
#include <iterator>

using namespace std;

CmdOpts g_the_opts;
const CmdOpts *g_opts = &g_the_opts;

int CmdOptsBase::init(int argc, const char **argv)
{
  int i;
  for(i = 1; i < argc; i++)
  {
    if(argv[i][0] != '-')
      break;
    auto it = m_opts.find(argv[i]);
    if(it == m_opts.end())
    {
      cerr << "Invalid argument: " << argv[i] << endl; 
      cerr << "Aborting" << endl;
      exit(1);
    }
    i += it->second(argv + i + 1);
  }
  return i;
}

void CmdOptsBase::runtime_args(const char *_args) const
{
  vector<string> ss;
  istringstream iss(_args);
  copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(ss));
  vector<const char *> args(ss.size() + 1);
  args[0] = "";
  for(int i = 0; i < ss.size(); i++)
    args[i + 1] = ss[i].c_str();
  const_cast<CmdOptsBase *>(this)->init(args.size(), args.data());
}

CmdOptsBase::CmdOptsBase()
{
  add_flag("help", [=]()
  {
    cerr << "Ray tracer. Usage: rt [ options ] [ lua-scene-file ]" << endl;
    cerr << endl;
    cerr << "Valid command-line arguments:" << endl;
    for(auto &opt : m_opts)
    {
      if(opt.first[1] == '-')
	cerr << "  " << opt.first << endl;
    }
    exit(0);
  }, 'h');
}

void CmdOptsBase::add_flag(const char *name, const function<void()> &cb, char shrt)
{
  add_parameter(name, shrt, [cb](const char **) { cb(); return 0; });
}

template<>
void CmdOptsBase::add_parameter<const char *>(const char *name, const function<void(const char *)> &cb, char shrt)
{
  add_parameter(name, shrt, [cb](const char **args) { cb(args[0]); return 1; });
}

template<>
void CmdOptsBase::add_parameter<int>(const char *name, const function<void(int)> &cb, char shrt)
{
  add_parameter(name, shrt, [cb](const char **args)
  {
    cb(atoi(args[0]));
    return 1;
  });
}

template<>
void CmdOptsBase::add_parameter<double>(const char *name, const function<void(double)> &cb, char shrt)
{
  add_parameter(name, shrt, [cb](const char **args)
  {
    cb(strtof(args[0], 0));
    return 1;
  });
}

void CmdOptsBase::add_parameter(const char *name, char shrt, const function<int(const char **)> &cb)
{
  m_opts.insert(OptMap::value_type(string("--") + name, cb));
  if(shrt != '\0')
    m_opts.insert(OptMap::value_type(string("-") + shrt, cb));
}

CmdOpts::CmdOpts()
{
  // Default values for program options.
  reproducible = false;
  aa = bv = true;
  aa_jitter = 0;
  aa_grid = 3;
  draw_aa = false;
  threads = 6;
  aa_threshold = 0.5;
  aa_jitter = 0.;
  caustic_num_photons = 0;
  caustic_num_neighbours = 5;
  gi_num_photons = 1000000;
  gi_num_neighbours = 100;
  caustic_pm_gran = 180;
  use_caustic_map = true;
  draw_caustic_map = false;
  draw_caustics_only = false;
  use_gi_map = false;
  draw_gi_map = false;
  draw_gi_only = false;
  soft_shadows = true;
  shadow_grid = 4;
  height = width = 0;
  miss_colour = Colour(1);
  bgstyle = BGS_COLOUR;
  unit_distance = 1;
  energy_fudge = 1;

  outs = &cout;
  errs = &cerr;
  dbgs = &clog;
  clog.clear(ios_base::badbit);

  // Add the arguments with their lambda handlers.
  add_flag("reproducible", [=]() { reproducible = true; });
  add_flag("disable-aa", [=]() { aa = false; });
  add_flag("disable-bv", [=]() { bv = false; });
  add_flag("disable-csg-bb", [=]() { disable_csg_bb = true; });
  add_flag("draw-aa", [=]() { draw_aa = true; });
  add_flag("timing", [=]() { timing = true; }, 't');
  add_flag("silent", [=]() { outs = &clog; }, 's');
  add_flag("stats", [=]() { stats = true; }, 'S');
  add_flag("verbose", [=]() { dbgs = &cout; }, 'v');
  add_flag("no-caustic-map", [=]() { use_caustic_map = false; });
  add_flag("draw-caustic-map", [=]() { draw_caustic_map = true; });
  add_flag("draw-caustics-only", [=]() { draw_caustics_only = true; });
  add_flag("debug", [=]() { dbgs = &cout; outs = &clog; threads = 1; }, 'd');
  add_flag("gi", [=]() { use_gi_map = true; });
  add_flag("draw-gi-only", [=]() { draw_gi_only = true; });
  add_flag("draw-gi-map", [=]() {draw_gi_map = true; });
  add_flag("no-soft-shadows", [=]() { soft_shadows = false; });
  add_flag("minres", [=] { height = width = 100; });
  add_flag("lores", [=] { height = width = 256; });
  add_flag("midres", [=] { height = width = 512; });
  add_flag("hires", [=] { height = width = 1024; });
  add_flag("rays", [=] { bgstyle = BGS_RAYS; });
  add_parameter<int>("aa-grid", [=](int g) { aa_grid = check_range(g, 1, "Invalid postive integer argument to aa-grid"); });
  add_parameter<int>("threads", [=](int t) { threads = check_range(t, 1, "Invalid positive integer argument to threads"); }, 'j');
  add_parameter<double>("aa-threshold", [=](double t) { aa_threshold = check_range(t, 0., "Jitter value must be non-negative double"); });
  add_parameter<int>("caustic-num-photons", [=](int n) { caustic_num_photons = check_range(n, 0, "# caustic photons must be non-negative"); });
  add_parameter<int>("caustic-num-neighbours", [=](int n) { caustic_num_neighbours = check_range(n, 0, "# caustic neighbours must be non-negative"); });
  add_parameter<int>("gi-num-photons", [=](int n) { gi_num_photons = check_range(n, 0, "# gi photons must be non-negative"); });
  add_parameter<int>("gi-num-neighbours", [=](int n) { gi_num_neighbours = check_range(n, 0, "# gi neighbours must be non-negative"); });
  add_parameter<int>("caustic-pm-gran", [=](int n) { caustic_pm_gran = check_range(n, 1, "Caustic granularity must be postive"); });
  add_parameter<int>("shadow-grid", [=](int n) { soft_shadows = true; shadow_grid = check_range(n, 1, "Shadow grid arg must be positive"); });
  add_parameter<double>("unit-distance", [=](double u) { unit_distance = check_range(u, 0., "unit distance must be non-negative"); });
  add_parameter<double>("energy-fudge", [=](double f) { energy_fudge = check_range(f, 0., "energy fudge must be non-negative"); });
}

void CmdOpts::set_miss_colour(const Colour &c) const
{
  CmdOpts *ths = const_cast<CmdOpts *>(this);
  ths->bgstyle = BGS_COLOUR;
  ths->miss_colour = c;
}
