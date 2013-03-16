#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <ctime>

class Timer
{
public:
  Timer() : m_running(false), m_name(0) {}
  ~Timer() { stop(); }

  void set_name(const char *name) { m_name = name; }
  void start();
  int stop();

  static void dump_timings();
private:
  bool m_running;
  time_t m_init;
  const char *m_name;
};

#define SCOPED_TIMER(s) \
  Timer _scoped_timer; \
  _scoped_timer.set_name(s); \
  _scoped_timer.start()

#endif
