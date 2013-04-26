/**
 * Multiple-consumer-style multithreading templates.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __THREADING_HPP__
#define __THREADING_HPP__

#include <mutex>
#include <functional>
#include <vector>

class Parallelize
{
public:
  Parallelize() : m_current(m_tasks.end()) {}

  void add_task(const std::function<void()> &t) { m_tasks.push_back(t); }
  void go();

private:
  typedef std::vector<std::function<void()>> TaskList;

  void worker();

  std::mutex m_mutex;
  TaskList m_tasks;
  TaskList::iterator m_current;
};

#endif
