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
#include <chrono>
#include <thread>
#include <future>
#include "cmdopts.hpp"

using namespace std;

template<typename T>
class Parallelize
{
public:
  typedef std::function<T()> func_t;

  Parallelize() {}

  future<T> add_task_future(const func_t &t)
  {
    unique_lock<mutex> lk(m_mutex);
    m_tasks.push(std::packaged_task<T()>(t));
    return m_tasks.back().get_future();
  }

  void add_task(const func_t &t)
  {
    unique_lock<mutex> lk(m_mutex);
    m_tasks.push(std::packaged_task<T()>(t));
  }

  void go()
  {
    vector<thread *> threads(GETOPT(threads));
    m_active = threads.size();
    for(auto &t : threads)
      t = new thread(mem_fn(&Parallelize::worker), this);

    for(auto t : threads)
    {
      t->join();
      delete t;
    }
  }

private:
  typedef std::packaged_task<T()> task_t;
  typedef std::queue<task_t> TaskList;

  void worker()
  {
    auto delay = std::chrono::milliseconds(5);
    bool active = true;
    for(;;)
    {
      std::packaged_task<T()> task;
      bool found = false;

      {
	unique_lock<mutex> lk(m_mutex);

	found = m_tasks.size() > 0;
	if(found)
	{
	  swap(task, m_tasks.front());
	  m_tasks.pop();
	  if(!active)
	  {
	    m_active++;
	    active = true;
	  }
	}
	else
	{
	  if(active)
	  {
	    m_active--;
	    active = false;
	  }
	  if(m_active == 0)
	    return;
	}
      }

      if(found)
	task();
      else
	this_thread::sleep_for(delay);
    }
  }


  std::mutex m_mutex;
  TaskList m_tasks;
  int m_active;
};

#endif
