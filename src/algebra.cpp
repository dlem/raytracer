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

#ifndef __ALGEBRA_HPP__
#define __ALGEBRA_HPP__

#include "algebra.hpp"
#include <cassert>
#include <limits>
#include "polyroots.hpp"

using namespace std;

double Vector3D::normalize()
{
  double denom = 1.0;
  double x = (v_[0] > 0.0) ? v_[0] : -v_[0];
  double y = (v_[1] > 0.0) ? v_[1] : -v_[1];
  double z = (v_[2] > 0.0) ? v_[2] : -v_[2];

  if(x > y) {
    if(x > z) {
      if(1.0 + x > 1.0) {
        y = y / x;
        z = z / x;
        denom = 1.0 / (x * sqrt(1.0 + y*y + z*z));
      }
    } else { /* z > x > y */ 
      if(1.0 + z > 1.0) {
        y = y / z;
        x = x / z;
        denom = 1.0 / (z * sqrt(1.0 + y*y + x*x));
      }
    }
  } else {
    if(y > z) {
      if(1.0 + y > 1.0) {
        z = z / y;
        x = x / y;
        denom = 1.0 / (y * sqrt(1.0 + z*z + x*x));
      }
    } else { /* x < y < z */
      if(1.0 + z > 1.0) {
        y = y / z;
        x = x / z;
        denom = 1.0 / (z * sqrt(1.0 + y*y + x*x));
      }
    }
  }

  if(1.0 + x + y + z > 1.0) {
    v_[0] *= denom;
    v_[1] *= denom;
    v_[2] *= denom;
    return 1.0 / denom;
  }

  return 0.0;
}

/*
 * Define some helper functions for matrix inversion.
 */

static void swaprows(Matrix4x4& a, size_t r1, size_t r2)
{
  std::swap(a[r1][0], a[r2][0]);
  std::swap(a[r1][1], a[r2][1]);
  std::swap(a[r1][2], a[r2][2]);
  std::swap(a[r1][3], a[r2][3]);
}

static void dividerow(Matrix4x4& a, size_t r, double fac)
{
  a[r][0] /= fac;
  a[r][1] /= fac;
  a[r][2] /= fac;
  a[r][3] /= fac;
}

static void submultrow(Matrix4x4& a, size_t dest, size_t src, double fac)
{
  a[dest][0] -= fac * a[src][0];
  a[dest][1] -= fac * a[src][1];
  a[dest][2] -= fac * a[src][2];
  a[dest][3] -= fac * a[src][3];
}

/*
 * invertMatrix
 *
 * I lifted this code from the skeleton code of a raytracer assignment
 * from a different school.  I taught that course too, so I figured it
 * would be okay.
 */
Matrix4x4 Matrix4x4::invert() const
{
  /* The algorithm is plain old Gauss-Jordan elimination 
     with partial pivoting. */

  Matrix4x4 a(*this);
  Matrix4x4 ret;

  /* Loop over cols of a from left to right, 
     eliminating above and below diag */

  /* Find largest pivot in column j among rows j..3 */
  for(size_t j = 0; j < 4; ++j) { 
    size_t i1 = j; /* Row with largest pivot candidate */
    for(size_t i = j + 1; i < 4; ++i) {
      if(fabs(a[i][j]) > fabs(a[i1][j])) {
        i1 = i;
      }
    }

    /* Swap rows i1 and j in a and ret to put pivot on diagonal */
    swaprows(a, i1, j);
    swaprows(ret, i1, j);

    /* Scale row j to have a unit diagonal */
    if(a[j][j] == 0.0) {
      // Theoretically throw an exception.
      return ret;
    }

    dividerow(ret, j, a[j][j]);
    dividerow(a, j, a[j][j]);

    /* Eliminate off-diagonal elems in col j of a, doing identical 
       ops to b */
    for(size_t i = 0; i < 4; ++i) {
      if(i != j) {
        submultrow(ret, i, j, a[i][j]);
        submultrow(a, i, j, a[i][j]);
      }
    }
  }

  return ret;
}

Matrix4x4 Matrix4x4::scale(const Vector3D &scale)
{
  Matrix4x4 s;
  s[0][0] = scale[0];
  s[1][1] = scale[1];
  s[2][2] = scale[2];
  return s;
}

Matrix4x4 Matrix4x4::translate(const Vector3D &displacement)
{
  Matrix4x4 t;
  t[0][3] = displacement[0];
  t[1][3] = displacement[1];
  t[2][3] = displacement[2];
  return t;
}

Matrix4x4 Matrix4x4::rotate(char axis, double angle)
{
  angle *= M_PI / 180;
  const double c = cos(angle), s = sin(angle);
  Matrix4x4 r;
  switch(axis)
  {
      case 'x': case 'X':
	  r[1][1] = c;
	  r[2][2] = c;
	  r[2][1] = s;
	  r[1][2] = -s;
	  break;
      case 'y': case 'Y':
	  r[0][0] = c;
	  r[2][2] = c;
	  r[0][2] = s;
	  r[2][0] = -s;
	  break;
      case 'z': case 'Z':
	  r[0][0] = c;
	  r[0][1] = -s;
	  r[1][0] = s;
	  r[1][1] = c;
	  break;
      default:
	  assert(0 && "Invalid axis");
	  break;
  };
  return r;
}

template<>
size_t Polynomial<2>::solve(double roots[2]) const
{
  return quadraticRoots(m_coefs[2], m_coefs[1], m_coefs[0], roots);
}

template<>
size_t Polynomial<1>::solve(double roots[2]) const
{
  if(m_coefs[1] == 0)
    return 0;
  roots[0] = -m_coefs[0] / m_coefs[1];
  return 1;
}

std::ostream &operator<<(std::ostream &os, Polynomial<2> &p)
{
  return os << p[2] << "t^2 + " << p[1] << "t + " << p[0];
}

#endif
