/**
 * Multiple-consumer-style multithreading templates.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#ifndef __THREADING_HPP__
#define __THREADING_HPP__

#include <thread>
#include <vector>
#include <cassert>

template<typename TIterator, typename TItem>
class ThreadsafeQueue
{
public:
  ThreadsafeQueue(TIterator start, TIterator end, const std::function<void(int)> &cb)
    : m_cur(start), m_end(end), m_cb(cb) {}

  bool consume(TItem &item)
  {
    std::unique_lock<std::mutex> lk(m_lock);
    if(m_cur == m_end)
      return false;
    m_cb(m_end - m_cur);
    item = *(m_cur++);
    return true;
  }

private:
  std::mutex m_lock;
  TIterator m_cur, m_end;
  std::function<void(int)> m_cb;
};

template<typename TIterator, typename TWork>
class Parfor
{
  typedef ThreadsafeQueue<TIterator, TWork> TQueue;
  typedef std::function<void(TWork &)> TFun;
public:
  Parfor(TIterator start, TIterator end, TFun fun)
    : m_start(start)
    , m_end(end)
    , m_fun(fun)
  {}

  void go(int nthreads, bool blocking, const std::function<void(int)> &progress)
  {
    TQueue q(m_start, m_end, progress);
    std::vector<std::thread *> threads;
    threads.resize(nthreads);
    for(int i = 0; i < nthreads; i++)
      threads[i] = new std::thread(worker, &q, m_fun);
    if(blocking)
      for(auto t = threads.begin(); t != threads.end(); t++)
	(*t)->join();
  }

private:

  static void worker(TQueue *q, TFun fun)
  {
    TWork work;
    while(q->consume(work))
      fun(work);
  }

  TIterator m_start, m_end;
  std::function<void(TWork &)> m_fun;
  std::vector<std::thread *> m_threads;
};

template<class TIterator, class TWork>
Parfor<TIterator, TWork> parfor(TIterator start, TIterator end,
			        std::function<void(TWork &)> fun)
{
  return Parfor<TIterator, TWork>(start, end, fun);
}

#endif
