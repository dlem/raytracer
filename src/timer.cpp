#include "timer.hpp"
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include "cmdopts.hpp"

#define CLOCK_TO_MILLIS(x) (int)((float)(x) / CLOCKS_PER_SEC * 1000)

using namespace std;

static mutex s_mutex;
static map<const string, time_t> s_timings;

void Timer::start()
{
  if(m_name)
    dbgs() << "Timer starting [ " << m_name << " ]" << endl;

  if(!m_running)
    m_init = clock();
}

int Timer::stop()
{
  m_running = false;
  const time_t end = clock(); 
  const time_t delta = end - m_init;
  const int rv = CLOCK_TO_MILLIS(delta);

  if(m_name)
  {
    unique_lock<mutex>(s_mutex);
    auto p = s_timings.find(m_name);
    if(p == s_timings.end())
      s_timings[m_name] = delta;
    else
      p->second += delta;

    dbgs() << "Timer finished [ " << m_name << " ] ";
    dbgs() << "(" << rv << "ms)" << endl;
  }

  return rv;
}

void Timer::dump_timings()
{
  unique_lock<mutex>(s_mutex);
  vector<pair<const string, time_t> *> timings(s_timings.size());

  int i = 0;
  for(auto &t : s_timings)
    timings[i++] = &t;

  sort(timings.begin(), timings.end(), []
      (pair<const string, time_t> *t1, pair<const string, time_t> *t2)
      { return t2->second < t1->second; });

  outs() << "Timings:" << endl;
  for(auto &t : timings)
    outs() << "  [ " << t->first << " ] "
	 << CLOCK_TO_MILLIS(t->second) << "ms" << endl;
}
