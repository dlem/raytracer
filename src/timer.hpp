/*
 * Timer class: useful for timing certain stages of rendering so that
 * performance can be recorded.
 *
 * ProgressTimer class: like the timer, but provides functionality for calling
 * into it with your current progress with a task, and prints progress and times
 * to the screen. Great for user-friendliness.
 *
 * Only one ProgressTimer can exist at a time, because they monopolize the output stream!
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/


#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <ctime>
#include <mutex>

class Timer
{
public:
  Timer(bool report = false) : m_running(false), m_report(report), m_name(0) {}
  virtual ~Timer() { stop(); }

  void set_name(const char *name) { m_name = name; }
  void start();
  int stop();
  void lap(int &user, int &real);

  static void dump_timings();
protected:
  bool m_running, m_report;
  time_t m_ustart, m_rstart;
  const char *m_name;
};

class ProgressTimer : public Timer
{
public:
  ProgressTimer(const char *name, int total);
  virtual ~ProgressTimer();
  void set_progress(int amount);
  void increment() { set_progress(m_progress + 1); }
  double get_progress();
  void report();

private:
  const int m_total;
  int m_progress;
};

#define SCOPED_TIMER(s) \
  Timer _scoped_timer; \
  _scoped_timer.set_name(s); \
  _scoped_timer.start()

#endif
