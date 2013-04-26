#include "threading.hpp"
#include <thread>
#include <cassert>
#include <iostream>
#include "cmdopts.hpp"

using namespace std;

void Parallelize::worker()
{
  for(;;)
  {
    TaskList::iterator task;

    {
      unique_lock<mutex> lk(m_mutex);
      if(m_current == m_tasks.end())
	return;

      task = m_current++;
    }

    (*task)();
  }
}

void Parallelize::go()
{
  m_current = m_tasks.begin();
  std::vector<thread *> threads(GETOPT(threads));
  for(auto &t : threads)
  {
    t = new thread(mem_fn(&Parallelize::worker), this);
  }

  for(auto t : threads)
  {
    t->join();
    delete t;
  }
}
