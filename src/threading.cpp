#include "threading.hpp"
#include <thread>
#include <cassert>
#include <iostream>
#include <chrono>
#include "cmdopts.hpp"

using namespace std;

void Parallelize::worker()
{
  auto delay = chrono::milliseconds(5);
  bool active = true;
  for(;;)
  {
    packaged_task<void()> task;
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

void Parallelize::go()
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

future<void> Parallelize::add_task(const std::function<void()> &t)
{
  m_tasks.push(packaged_task<void()>(t));
  return m_tasks.back().get_future();
}
