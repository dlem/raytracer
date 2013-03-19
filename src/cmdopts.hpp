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

private:
  void add_parameter(const char *name, char shrt, const std::function<int(const char **)> &handler);

  typedef std::map<const std::string, std::function<int(const char **handler)>> OptMap;
  OptMap m_opts;
};

class CmdOpts : public CmdOptsBase
{
public:
  CmdOpts();
  bool aa, bv, draw_aa, timing;
  bool stats, use_caustic_map, draw_caustic_map;
  bool draw_caustic_pm;
  bool draw_caustics_only;
  bool draw_gi_map;
  bool draw_gi_only;
  bool use_gi_map;
  unsigned aa_grid, threads;
  double aa_threshold, aa_jitter;
  unsigned caustic_num_photons;
  unsigned caustic_pm_gran;
  unsigned caustic_num_neighbours;
  std::ostream *outs;
  std::ostream *dbgs;
  std::ostream *errs;
};

extern const CmdOpts *g_opts;

static inline std::ostream &dbgs() { return *g_opts->dbgs; }
static inline std::ostream &outs() { return *g_opts->outs; }
static inline std::ostream &errs() { return *g_opts->errs; }

#define GETOPT(opt) (g_opts->opt)

#endif
