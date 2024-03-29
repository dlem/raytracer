//---------------------------------------------------------------------------
//
// CS488 -- Introduction to Computer Graphics
//
// algebra.hpp/algebra.cpp
//
// Classes and functions for manipulating points, vectors, matrices, 
// and colours.  You probably won't need to modify anything in these
// two files.
//
// University of Waterloo Computer Graphics Lab / 2003
//
// Modified by Daniel Lemmond, dlemmond, 20302247.
//
//---------------------------------------------------------------------------

#ifndef CS488_ALGEBRA_HPP
#define CS488_ALGEBRA_HPP

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cassert>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const double phi = 1.61803398875;

#define NUMELMS(x) (sizeof(x)/sizeof(*(x)))

template<typename T>
T sqr(T x) { return x * x; }

template<typename T>
T clamp(T val, T lo, T hi)
{ return std::max(lo, std::min(hi, val)); }

template<typename T>
bool inrange(T val, T lo, T hi)
{ return val >= lo && val <= hi; }

template<typename T>
T safe_div(T x, T y)
{
  return y == 0 ? 0 : x / y;
}

template<typename T>
bool approx(T v1, T v2)
{
  const double epsilon = 0.0001;
  return inrange(v1, v2 - epsilon, v2 + epsilon);
}

template<typename T>
T norm(T tin)
{
  T tout = tin;
  tout.normalize();
  return tout;
}

template<typename T>
bool normalized(T t)
{
  return approx(t.length(), 1.);
}

class Point2D
{
public:
  Point2D()
  {
    v_[0] = 0.0;
    v_[1] = 0.0;
  }
  Point2D(double x, double y)
  { 
    v_[0] = x;
    v_[1] = y;
  }
  Point2D(const Point2D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
  }

  Point2D& operator =(const Point2D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    return *this;
  }

  double& operator[](size_t idx) 
  {
    return v_[ idx ];
  }
  double operator[](size_t idx) const 
  {
    return v_[ idx ];
  }

private:
  double v_[2];
};

class Point3D
{
public:
  Point3D()
  {
    v_[0] = 0.0;
    v_[1] = 0.0;
    v_[2] = 0.0;
  }
  Point3D(double x, double y, double z)
  { 
    v_[0] = x;
    v_[1] = y;
    v_[2] = z;
  }
  Point3D(const Point3D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    v_[2] = other.v_[2];
  }

  Point3D& operator =(const Point3D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    v_[2] = other.v_[2];
    return *this;
  }

  double& operator[](size_t idx) 
  {
    return v_[ idx ];
  }
  double operator[](size_t idx) const 
  {
    return v_[ idx ];
  }

private:
  double v_[3];
};

class Vector3D
{
public:
  Vector3D()
  {
    v_[0] = 0.0;
    v_[1] = 0.0;
    v_[2] = 0.0;
  }
  Vector3D(double x, double y, double z)
  { 
    v_[0] = x;
    v_[1] = y;
    v_[2] = z;
  }
  Vector3D(const Vector3D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    v_[2] = other.v_[2];
  }

  Vector3D& operator =(const Vector3D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    v_[2] = other.v_[2];
    return *this;
  }

  double& operator[](size_t idx) 
  {
    return v_[ idx ];
  }
  double operator[](size_t idx) const 
  {
    return v_[ idx ];
  }

  double dot(const Vector3D& other) const
  {
    return v_[0]*other.v_[0] + v_[1]*other.v_[1] + v_[2]*other.v_[2];
  }

  double length2() const
  {
    return v_[0]*v_[0] + v_[1]*v_[1] + v_[2]*v_[2];
  }
  double length() const
  {
    return sqrt(length2());
  }

  double normalize();

  Vector3D cross(const Vector3D& other) const
  {
    return Vector3D(
                    v_[1]*other[2] - v_[2]*other[1],
                    v_[2]*other[0] - v_[0]*other[2],
                    v_[0]*other[1] - v_[1]*other[0]);
  }

private:
  double v_[3];
};

inline Point3D cw_max(const Point3D &p1, const Point3D &p2)
{
  return Point3D(std::max(p1[0], p2[0]),
		 std::max(p1[1], p2[1]),
		 std::max(p1[2], p2[2]));
}

inline Point3D cw_min(const Point3D &p1, const Point3D &p2)
{
  return Point3D(std::min(p1[0], p2[0]),
		 std::min(p1[1], p2[1]),
		 std::min(p1[2], p2[2]));
}

inline Vector3D operator *(double s, const Vector3D& v)
{
  return Vector3D(s*v[0], s*v[1], s*v[2]);
}

inline Vector3D operator +(const Vector3D& a, const Vector3D& b)
{
  return Vector3D(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
}

inline Point3D operator +(const Point3D& a, const Vector3D& b)
{
  return Point3D(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
}

inline Vector3D operator -(const Point3D& a, const Point3D& b)
{
  return Vector3D(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

inline Vector3D operator -(const Vector3D& a, const Vector3D& b)
{
  return Vector3D(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

inline Vector3D operator -(const Vector3D& a)
{
  return Vector3D(-a[0], -a[1], -a[2]);
}

inline Point3D operator -(const Point3D& a, const Vector3D& b)
{
  return Point3D(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

inline Vector3D cross(const Vector3D& a, const Vector3D& b) 
{
  return a.cross(b);
}

inline std::ostream& operator <<(std::ostream& os, const Point2D& p)
{
  return os << "p<" << p[0] << "," << p[1] << ">";
}

inline std::ostream& operator <<(std::ostream& os, const Point3D& p)
{
  return os << "p<" << p[0] << "," << p[1] << "," << p[2] << ">";
}

inline std::ostream& operator <<(std::ostream& os, const Vector3D& v)
{
  return os << "v<" << v[0] << "," << v[1] << "," << v[2] << ">";
}

class Matrix4x4;

class Vector4D
{
public:
  Vector4D()
  {
    v_[0] = 0.0;
    v_[1] = 0.0;
    v_[2] = 0.0;
    v_[3] = 0.0;
  }
  Vector4D(double x, double y, double z, double w)
  { 
    v_[0] = x;
    v_[1] = y;
    v_[2] = z;
    v_[3] = w;
  }
  Vector4D(const Vector4D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    v_[2] = other.v_[2];
    v_[3] = other.v_[3];
  }

  Vector4D& operator =(const Vector4D& other)
  {
    v_[0] = other.v_[0];
    v_[1] = other.v_[1];
    v_[2] = other.v_[2];
    v_[3] = other.v_[3];
    return *this;
  }

  double& operator[](size_t idx) 
  {
    return v_[ idx ];
  }
  double operator[](size_t idx) const 
  {
    return v_[ idx ];
  }

private:
  double v_[4];
};

class Matrix4x4
{
public:
  Matrix4x4()
  {
    // Construct an identity matrix
    std::fill(v_, v_+16, 0.0);
    v_[0] = 1.0;
    v_[5] = 1.0;
    v_[10] = 1.0;
    v_[15] = 1.0;
  }
  Matrix4x4(const Matrix4x4& other)
  {
    std::copy(other.v_, other.v_+16, v_);
  }
  Matrix4x4(const Vector4D row1, const Vector4D row2, const Vector4D row3, 
             const Vector4D row4)
  {
    v_[0] = row1[0]; 
    v_[1] = row1[1]; 
    v_[2] = row1[2]; 
    v_[3] = row1[3]; 

    v_[4] = row2[0]; 
    v_[5] = row2[1]; 
    v_[6] = row2[2]; 
    v_[7] = row2[3]; 

    v_[8] = row3[0]; 
    v_[9] = row3[1]; 
    v_[10] = row3[2]; 
    v_[11] = row3[3]; 

    v_[12] = row4[0]; 
    v_[13] = row4[1]; 
    v_[14] = row4[2]; 
    v_[15] = row4[3]; 
  }
  Matrix4x4(const Vector3D row1, const Vector3D row2, const Vector3D row3)
  {
    v_[0] = row1[0]; 
    v_[1] = row1[1]; 
    v_[2] = row1[2]; 
    v_[3] = 0;

    v_[4] = row2[0]; 
    v_[5] = row2[1]; 
    v_[6] = row2[2]; 
    v_[7] = 0;

    v_[8] = row3[0]; 
    v_[9] = row3[1]; 
    v_[10] = row3[2]; 
    v_[11] = 0;

    v_[12] = v_[13] = v_[14] = 0;
    v_[15] = 1;
  }
  Matrix4x4(double *vals)
  {
    std::copy(vals, vals + 16, (double*)v_);
  }

  Matrix4x4& operator=(const Matrix4x4& other)
  {
    std::copy(other.v_, other.v_+16, v_);
    return *this;
  }

  Vector4D getRow(size_t row) const
  {
    return Vector4D(v_[4*row], v_[4*row+1], v_[4*row+2], v_[4*row+3]);
  }
  double *getRow(size_t row) 
  {
    return (double*)v_ + 4*row;
  }

  Vector4D getColumn(size_t col) const
  {
    return Vector4D(v_[col], v_[4+col], v_[8+col], v_[12+col]);
  }

  Vector4D operator[](size_t row) const
  {
    return getRow(row);
  }
  double *operator[](size_t row) 
  {
    return getRow(row);
  }

  Matrix4x4 transpose() const
  {
    return Matrix4x4(getColumn(0), getColumn(1), 
                      getColumn(2), getColumn(3));
  }

  Matrix4x4 linear() const
  {
    Matrix4x4 rv = *this;
    rv[0][3] = 0;
    rv[1][3] = 0;
    rv[2][3] = 0;
    rv[3][3] = 0;
    rv[3][1] = 0;
    rv[3][2] = 0;
    return rv;
  }

  Matrix4x4 invert() const;

  const double *begin() const
  {
    return (double*)v_;
  }
  const double *end() const
  {
    return begin() + 16;
  }

  static Matrix4x4 scale(const Vector3D &scale);
  static Matrix4x4 translate(const Vector3D &displacement);

  // Rotatation matrix about the x, y, or z axis.
  static Matrix4x4 rotate(char axis, double angle);

  // Rotation matrix about an arbitrary line.
  static Matrix4x4 rotate(const Point3D &pt, const Vector3D &dir, double angle);
		
private:
  double v_[16];
};

inline Matrix4x4 operator *(const Matrix4x4& a, const Matrix4x4& b)
{
  Matrix4x4 ret;

  for(size_t i = 0; i < 4; ++i) {
    Vector4D row = a.getRow(i);
		
    for(size_t j = 0; j < 4; ++j) {
      ret[i][j] = row[0] * b[0][j] + row[1] * b[1][j] + 
        row[2] * b[2][j] + row[3] * b[3][j];
    }
  }

  return ret;
}

inline Vector3D operator *(const Matrix4x4& M, const Vector3D& v)
{
  return Vector3D(
                  v[0] * M[0][0] + v[1] * M[0][1] + v[2] * M[0][2],
                  v[0] * M[1][0] + v[1] * M[1][1] + v[2] * M[1][2],
                  v[0] * M[2][0] + v[1] * M[2][1] + v[2] * M[2][2]);
}

inline Point3D operator *(const Matrix4x4& M, const Point3D& p)
{
  return Point3D(
                 p[0] * M[0][0] + p[1] * M[0][1] + p[2] * M[0][2] + M[0][3],
                 p[0] * M[1][0] + p[1] * M[1][1] + p[2] * M[1][2] + M[1][3],
                 p[0] * M[2][0] + p[1] * M[2][1] + p[2] * M[2][2] + M[2][3]);
}

inline Vector3D transNorm(const Matrix4x4& M, const Vector3D& n)
{
  return Vector3D(
                  n[0] * M[0][0] + n[1] * M[1][0] + n[2] * M[2][0],
                  n[0] * M[0][1] + n[1] * M[1][1] + n[2] * M[2][1],
                  n[0] * M[0][2] + n[1] * M[1][2] + n[2] * M[2][2]);
}

inline std::ostream& operator <<(std::ostream& os, const Matrix4x4& M)
{
  return os << "[" << M[0][0] << " " << M[0][1] << " " 
            << M[0][2] << " " << M[0][3] << "]" << std::endl
            << "[" << M[1][0] << " " << M[1][1] << " " 
            << M[1][2] << " " << M[1][3] << "]" << std::endl
            << "[" << M[2][0] << " " << M[2][1] << " " 
            << M[2][2] << " " << M[2][3] << "]" << std::endl
            << "[" << M[3][0] << " " << M[3][1] << " " 
            << M[3][2] << " " << M[3][3] << "]";
}

class Colour
{
public:
  Colour(double r, double g, double b)
    : r_(r)
    , g_(g)
    , b_(b)
  {}
  Colour(double c = 0.)
    : r_(c)
    , g_(c)
    , b_(c)
  {}
  Colour(const Colour& other)
    : r_(other.r_)
    , g_(other.g_)
    , b_(other.b_)
  {}

  Colour& operator =(const Colour& other)
  {
    r_ = other.r_;
    g_ = other.g_;
    b_ = other.b_;
    return *this;
  }

  double R() const 
  { 
    return r_;
  }
  double G() const 
  { 
    return g_;
  }
  double B() const 
  { 
    return b_;
  }
  double Y() const
  {
    // From Wikipedia.
    return 0.299 * R() + 0.587 * G() + 0.114 * B();
  }
  double normalize()
  {
    double len = sqrt(r_ * r_ + g_ * g_ + b_ * b_);
    if(len > 0)
    {
      r_ /= len;
      g_ /= len;
      b_ /= len;
    }
    return len;
  }
  void pow(double x)
  {
    r_ = ::pow(r_, x);
    g_ = ::pow(g_, x);
    b_ = ::pow(b_, x);
  }

private:
  double r_;
  double g_;
  double b_;
};

inline Colour operator *(double s, const Colour& a)
{
  return Colour(s*a.R(), s*a.G(), s*a.B());
}

inline Colour operator *(const Colour& a, const Colour& b)
{
  return Colour(a.R()*b.R(), a.G()*b.G(), a.B()*b.B());
}

inline Colour operator +(const Colour& a, const Colour& b)
{
  return Colour(a.R()+b.R(), a.G()+b.G(), a.B()+b.B());
}

inline Colour operator+=(Colour &a, const Colour &b)
{
  return (a = a + b);
}

inline std::ostream& operator <<(std::ostream& os, const Colour& c)
{
  return os << "c<" << c.R() << "," << c.G() << "," << c.B() << ">";
}

template<int N>
class Polynomial
{
  friend std::ostream &operator<<(std::ostream &, Polynomial<2> &);

public:
  Polynomial(const std::initializer_list<double> &coefs)
  {
    assert(coefs.size() == NUMELMS(m_coefs));
    int i = 0;
    for(auto coef : coefs)
      m_coefs[i++] = coef;
  }

  Polynomial()
  {
    memset(this, 0, sizeof(*this));
  }

  double operator[](int n) const { return m_coefs[n]; }
  double &operator[](int n) { return m_coefs[n]; }

  Polynomial<N> operator+(const Polynomial<N> &other) const
  {
    Polynomial<N> out;
    for(int i = 0; i < NUMELMS(m_coefs); i++)
      out[i] = m_coefs[i] + other[i];
    return out;
  }

  inline Polynomial<N> operator*(const Polynomial<N> &xs2) const;

  Polynomial<N> operator*(double c) const
  {
    Polynomial<N> out;
    for(int i = 0; i < NUMELMS(m_coefs); i++)
      out[i] = c * m_coefs[i];
    return out;
  }

  Polynomial<N> operator+(double c) const
  {
    Polynomial<N> out = *this;
    out[0] += c;
    return out;
  }

  Polynomial<N> substitute(const Polynomial<N> &subbee) const
  {
    Polynomial<N> factor = Polynomial<N>({1, 0, 0, 0});
    Polynomial<N> rv;
    for(int i = 0; i < NUMELMS(m_coefs); i++)
    {
      rv[i] = m_coefs[i] * factor;
      factor = factor * subbee;
    }
    return rv;
  }

  size_t solve(double roots[N]) const;
  double eval(double t) const
  {
    double rv = 0;
    double factor = 1;
    for(double coef : m_coefs)
    {
      rv += factor * coef;
      factor *= t;
    }
    return rv;
  }

private:
  // m_coefs[0] ~ coef for x^0.
  double m_coefs[N + 1];
};

template<>
inline Polynomial<2> Polynomial<2>::operator *(const Polynomial<2> &other) const
{
  Polynomial<2> out;
  out[0] = m_coefs[0] * other.m_coefs[0];
  out[1] = m_coefs[0] * other.m_coefs[1] + m_coefs[1] * other.m_coefs[0];
  out[2] = m_coefs[1] * other.m_coefs[1];
  return out;
}

template<int N>
inline Polynomial<N> Polynomial<N>::operator *(const Polynomial<N> &xs2) const
{
  Polynomial<N> out;
  auto &xs1 = *this;
  for(int n = 0; n < NUMELMS(m_coefs); n++)
    for(int m = 0; m <= n; m++)
      out[n] += xs1[m] * xs2[n - m];
  return out;
}

template<int N>
Polynomial<N> operator*(double c, const Polynomial<N> &p)
{
  return p * c;
}

template<int N>
Polynomial<N> operator+(double c, const Polynomial<N> &p)
{
  return p + c;
}

std::ostream &operator<<(std::ostream &os, Polynomial<2> &p);


#endif // CS488_ALGEBRA_HPP
