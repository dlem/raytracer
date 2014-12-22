/**
 * Code involving program options. Get an option using the GETOPT macro.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __CMDOPTS_HPP__
#define __CMDOPTS_HPP__

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include "algebra.hpp"

// Base class for command-line argument container. Derived is the one who
// defines the actual options. Derived must add arguments by passing function
// handlers to the add_parameter and add_flag methods.
class CmdOptsBase
{
public:
  CmdOptsBase();
  virtual ~CmdOptsBase() {}

  // Add an arg which accepts a single value of type T. Only instantiated for
  // certain types.
  template<typename T>
  void add_parameter(const char *name, const std::function<void(T)> &handler, char shrt = '\0');

  // Add an flag argument (accepts no values).
  void add_flag(const char *name, const std::function<void()> &handler, char shrt = '\0');

  // Helpers for checking the range of an argument.
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

  // Called by main with command-line flags. Returns the index of the first
  // positional argument.
  int init(int argc, const char **argv);

  // Update the structure with an argument string (which may contain many
  // individual arguments) while the program is running.
  void runtime_args(const char *args) const;

private:
  void add_parameter(const char *name, char shrt, const std::function<int(const char **)> &handler);

  typedef std::map<const std::string, std::function<int(const char **handler)>> OptMap;
  OptMap m_opts;
};

enum BGSTYLE
{
  // Background is a solid colour.
  BGS_COLOUR,
  // Background is the rays BG.
  BGS_RAYS,
};

class CmdOpts : public CmdOptsBase
{
public:
  CmdOpts();

  // These are the program options.
  bool reproducible;
  bool aa, bv, draw_aa, timing;
  bool disable_csg_bb;
  bool stats, use_caustic_map, draw_caustic_map;
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

// The three output streams.
static inline std::ostream &dbgs() { return *g_opts->dbgs; }
static inline std::ostream &outs() { return *g_opts->outs; }
static inline std::ostream &errs() { return *g_opts->errs; }

// The macro used to get arguments. 'opt' must be a member of the CmdOpts
// struct.
#define GETOPT(opt) (g_opts->opt)

static inline unsigned get_rng_seed(const unsigned entropy = 0)
{
  unsigned seed = entropy;

  if (!GETOPT(reproducible))
  {
    seed ^= time(0);
  }

  return seed;
}

#endif
