/**
 * Horrible command-line parsing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#ifndef __CMDOPTS_HPP__
#define __CMDOPTS_HPP__

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include "algebra.hpp"

class CmdOptsBase
{
public:
  CmdOptsBase();
  virtual ~CmdOptsBase() {}

  template<typename T>
  void add_parameter(const char *name, const std::function<void(T)> &handler, char shrt = '\0');

  void add_flag(const char *name, const std::function<void()> &handler, char shrt = '\0');

  template<typename T>
  T check_range(T val, T lo, T hi, const char *msg)
  {
    if(val < lo || val > hi)
    {
      std::cerr << msg << std::endl;
      std::cerr << "Aborting" << std::endl;
      exit(1);
    }
    return val;
  }

  template<typename T>
  T check_range(T val, T lo, const char *msg)
  {
    if(val < lo)
    {
      std::cerr << msg << std::endl;
      std::cerr << "Aborting" << std::endl;
      exit(1);
    }
    return val;
  }

  int init(int argc, const char **argv);
  void runtime_args(const char *args) const;

private:
  void add_parameter(const char *name, char shrt, const std::function<int(const char **)> &handler);

  typedef std::map<const std::string, std::function<int(const char **handler)>> OptMap;
  OptMap m_opts;
};

enum BGSTYLE
{
  BGS_COLOUR,
  BGS_RAYS,
};

class CmdOpts : public CmdOptsBase
{
public:
  CmdOpts();
  bool aa, bv, draw_aa, timing;
  bool disable_csg_bb;
  bool stats, use_caustic_map, draw_caustic_map;
  bool draw_caustic_prm;
  bool disable_caustic_prm;
  bool draw_caustics_only;
  bool draw_gi_map;
  bool draw_gi_only;
  bool use_gi_map;
  bool soft_shadows;
  Colour miss_colour;
  unsigned aa_grid, threads;
  double aa_threshold, aa_jitter;
  unsigned caustic_num_photons;
  unsigned gi_num_photons;
  unsigned caustic_pm_gran;
  unsigned caustic_num_neighbours;
  unsigned gi_num_neighbours;
  unsigned shadow_grid;
  unsigned height, width;
  double unit_distance;
  double energy_fudge;
  BGSTYLE bgstyle;
  std::ostream *outs;
  std::ostream *dbgs;
  std::ostream *errs;

  void set_miss_colour(const Colour &c) const;
};

extern const CmdOpts *g_opts;

static inline std::ostream &dbgs() { return *g_opts->dbgs; }
static inline std::ostream &outs() { return *g_opts->outs; }
static inline std::ostream &errs() { return *g_opts->errs; }

#define GETOPT(opt) (g_opts->opt)

#endif
