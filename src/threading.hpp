/**
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __THREADING_HPP__
#define __THREADING_HPP__

#include <mutex>
#include <functional>
#include <vector>
#include <queue>
#include <future>

class Parallelize
{
public:
  typedef std::function<void()> func_t;

  Parallelize() {}

  std::future<void> add_task(const func_t &t);
  void go();

private:
  typedef std::packaged_task<void()> task_t;
  typedef std::queue<task_t> TaskList;

  void worker();

  std::mutex m_mutex;
  TaskList m_tasks;
  int m_active;
};

#endif
