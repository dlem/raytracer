#include "timer.hpp"
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <thread>
#include <ctime>
#include <sys/time.h>
#include "cmdopts.hpp"

#define CLOCK_TO_MILLIS(x) (int)((float)(x) / CLOCKS_PER_SEC * 1000)

using namespace std;

static mutex s_mutex; // guards access to s_timings
static map<const string, time_t> s_timings;

int current_time_ms()
{
  struct timeval tv; 
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void Timer::start()
{
  if(m_name && m_report)
    dbgs() << "Timer starting [ " << m_name << " ]" << endl;

  if(!m_running)
  {
    m_ustart = clock();
    m_rstart = current_time_ms();
  }
}

int Timer::stop()
{
  m_running = false;
  const time_t uend = clock(); 
  const time_t udelta = uend - m_ustart;
  const time_t rend = current_time_ms();
  const time_t rdelta = rend - m_rstart;
  const int rv = CLOCK_TO_MILLIS(rdelta);

  if(m_name)
  {
    unique_lock<mutex>(s_mutex);
    auto p = s_timings.find(m_name);
    if(p == s_timings.end())
      s_timings[m_name] = udelta;
    else
      p->second += udelta;

    if(m_report)
    {
      dbgs() << "Timer finished [ " << m_name << " ] ";
      dbgs() << "(Real: " << rv << "ms, User: " << udelta << "ms)" << endl;
    }
  }

  return rv;
}

void Timer::lap(int &user, int &real)
{
  const time_t unow = clock();
  const time_t rnow = current_time_ms();
  user = CLOCK_TO_MILLIS(unow - m_ustart);
  real = rnow - m_rstart;
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

static ProgressTimer *s_pt = 0;
static thread *s_reporter = 0;
// Guards access to the state of s_pt.
static mutex s_pmutex;

// Loop forever, sleeping and occassionally calling the report method on the
// current ProgressTimer if there is one.
void reporter()
{
  for(;;)
  {
    this_thread::sleep_for(chrono::milliseconds(10));
    unique_lock<mutex> lk(s_pmutex); 
    if(s_pt)
    {
      s_pt->report();
    }
  }
}

// We assume that there's only one ProgressTimer at a time.
ProgressTimer::ProgressTimer(const char *name, int total)
  : m_total(total)
  , m_progress(0)
{
  set_name(name);
  assert(s_pt == 0);
  start();
  report();
  s_pt = this;

  // Creates a reporter if there isn't already one.
  if(!s_reporter)
  {
    s_reporter = new thread(reporter);
  }
}

ProgressTimer::~ProgressTimer()
{
  unique_lock<mutex> lk(s_pmutex);
  report();
  outs() << endl;
  s_pt = 0;
}

void ProgressTimer::set_progress(int amount)
{
  unique_lock<mutex> lk(s_pmutex);
  m_progress = amount;
}

double ProgressTimer::get_progress()
{
  return m_progress / (double)max(m_total, 1);
}

void ProgressTimer::report()
{
  int real, user;
  lap(user, real);

  // Report time and progress.
  outs() << "\033[1G"; // Go to start of current line.
  outs() << setw(6) << setprecision(2) << fixed << 100 * get_progress()
	 << "% " << setw(25) << m_name << " (user" << setw(8) << user
	 << "ms, real: " << setw(4) << real << "ms)";
}
