#include "stats.hpp"
#include <mutex>
#include <vector>
#include <functional>
#include "cmdopts.hpp"

using namespace std;

typedef pair<const char *, function<void(ostream &)>> StatPair;

static mutex s_mutex;
static vector<StatPair> s_stats;

template<typename T>
void add_stat(const char *name, const T &t)
{
  unique_lock<mutex> lk(s_mutex);
  s_stats.push_back(StatPair(name, [t](ostream &os)
  {
    os << t;
  }));
}

#define STATS_INSTANTIATE(Ty) \
  template void add_stat<Ty>(const char *, const Ty &)

STATS_INSTANTIATE(int);
STATS_INSTANTIATE(double);
STATS_INSTANTIATE(unsigned);

void dump_stats()
{
  unique_lock<mutex> lk(s_mutex);

  outs() << "Stats:";
  for(auto &s : s_stats)
  {
    outs() << "  [ " << s.first << " ]";
    s.second(outs());
    outs() << endl;
  }
}
