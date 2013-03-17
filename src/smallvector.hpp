#ifndef __SMALL_VECTOR_HPP__
#define __SMALL_VECTOR_HPP__

#include <vector>

template<typename T, unsigned N>
class SmallVector
{
public:
  typedef T value_type; 
  typedef T &reference;
  typedef const T &const_reference;
  typedef size_t size_type;
  
  SmallVector()
    : m_size(0)
    , m_data(m_array)
  {}

  void push_back(const T &t)
  {
    const int idx = m_size;
    resize(idx + 1);
    m_data[idx] = t;
  }

  T *begin() { return m_data; }
  T *end() { return m_data + m_size; }

  void pop_back() { resize(m_size - 1); }

  T &front() { return *m_data; }
  const T &front() const { return *m_data; }

  T &operator[](int idx) { return m_data[idx]; }
  const T &operator[](int idx) const { return m_data[idx]; }

  size_t size() const { return m_size; }
  bool empty() const { return size() <= 0; }

  void resize(int size)
  {
    if(size > N && m_data != m_vector.data())
    {
      m_vector.assign(m_data, m_data + m_size);
      m_data = m_vector.data();
    }
    else if(m_data == m_vector.data())
      m_vector.resize(size);

    m_size = size;
  }

private:
  T *m_data;
  std::vector<T> m_vector;
  size_t m_size;
  T m_array[N];
};

#endif
