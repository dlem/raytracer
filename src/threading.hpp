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

extern __thread unsigned int g_worker_thread_num;

template<typename T>
class Parallelize
{
public:
  typedef std::function<T()> func_t;

  Parallelize() {}

  future<T> add_task_future(const func_t &t)
  {
    lock_guard<mutex> lk(m_mutex);
    m_tasks.push(std::packaged_task<T()>(t));
    return m_tasks.back().get_future();
  }

  void add_task(const func_t &t)
  {
    lock_guard<mutex> lk(m_mutex);
    m_tasks.push(std::packaged_task<T()>(t));
  }

  void go()
  {
    vector<thread *> threads(GETOPT(threads));
    m_active = threads.size();
    for(int i = 0; i < threads.size(); i++)
      threads[i] = new thread(mem_fn(&Parallelize::worker), this, i);

    for(auto t : threads)
    {
      t->join();
      delete t;
    }
  }

private:
  typedef std::packaged_task<T()> task_t;
  typedef std::queue<task_t> TaskList;

  void worker(int thread_num)
  {
    g_worker_thread_num = thread_num;
    auto delay = std::chrono::milliseconds(5);
    bool active = true;
    for(;;)
    {
      std::packaged_task<T()> task;
      bool found = false;

      {
	lock_guard<mutex> lk(m_mutex);

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

template<typename T>
class ThreadLocal
{
public:
  typedef std::function<T *()> factory_t;

  ThreadLocal(const factory_t &factory)
    : m_factory(factory)
  {}

  ~ThreadLocal()
  {
    for(auto t : m_ts)
      delete t;
  }

  T &get()
  {
    lock_guard<mutex> lk(m_mutex);
    if(m_ts.size() < g_worker_thread_num + 1)
      m_ts.resize((g_worker_thread_num + 1) * 2);

    if(!m_ts[g_worker_thread_num])
      m_ts[g_worker_thread_num] = m_factory();

    return *m_ts[g_worker_thread_num];
  }

private:
  vector<T *> m_ts;
  factory_t m_factory;
  mutex m_mutex;
};

#endif
