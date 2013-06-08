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

// Stopwatch; call start and stop to manage it; call lap to get the current user
// and real times. Also reports times if you give it a name and set report, and
// stores the times in a name-indexed map. Dumps the map via the dump_timings
// method.
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

// Timer that monopolizes cout by constantly printing how long the task is
// taking as well as the progress, taking advantage of ANSI codes to make it
// look good.
class ProgressTimer : public Timer
{
public:
  // Create timer with given name and 'total' for total progress.
  ProgressTimer(const char *name, int total);
  virtual ~ProgressTimer();

  // Set the current progress that'll get printed to cout.
  void set_progress(int amount);

  // Increment current progress by one.
  void increment(int amount = 1);
  double get_progress();

  // Print time/progress to cout.
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
