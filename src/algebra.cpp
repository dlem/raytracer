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

template<int Face>
double t_for_face(const Point3D &eye, const Vector3D &ray, const Point3D &mins, const Point3D &maxes)
{
  const int coord = Face / 2;
  const double goal = Face % 2 == 0 ? maxes[coord] : mins[coord];
  const double at = eye[coord];
  const double dir = ray[coord];

  if(dir == 0)
    return numeric_limits<double>::max();

  return (goal - at) / dir;
}

template<int FACE>
bool is_inside(const Point3D &p, const Point3D &mins, const Point3D &maxes)
{
  const int coord = FACE / 2;
  const int c1 = (coord + 1) % 3;
  const int c2 = (coord + 2) % 3;
  return p[c1] >= mins[c1] && p[c1] <= maxes[c1]
      && p[c2] >= mins[c2] && p[c2] <= maxes[c2];
}

void cube_uv(int face, const Point3D &p, Point2D &uv, Vector3D &u, Vector3D &v)
{
  const int coord = face / 2;
  const int c1 = (coord + 1) % 3;
  const int c2 = (coord + 2) % 3;
  switch(face)
  {
    case 0:
      uv = Point2D(2/4., 1/3.);
      u = Vector3D(0, -1, 0);
      v = Vector3D(0, 0, -1);
      break;
    case 1:
      uv = Point2D(0, 1/3.);
      u = Vector3D(0, 1, 0);
      v = Vector3D(0, 0, -1);
      break;
    case 2:
      uv = Point2D(1/3., 1/4.);
      u = Vector3D(1, 0, 0);
      v = Vector3D(0, 0, -1);
      break;
    case 3:
      uv = Point2D(3/4., 1/3.);
      u = Vector3D(-1, 0, 0);
      v = Vector3D(0, 0, -1);
      break;
    case 4:
      uv = Point2D(1/4., 2/3.);
      u = Vector3D(1, 0, 0);
      v = Vector3D(0, -1, 0);
      break;
    case 5:
      uv = Point2D(1/4., 0);
      u = Vector3D(1, 0, 0);
      v = Vector3D(0, -1, 0);
      break;
    default: assert(0); break;
  }
  uv[c1] += p[c1];
  uv[c2] += p[c2];
}

bool axis_aligned_box_check(const Point3D &eye, const Point3D &ray_end,
			    const Point3D &mins, const Point3D &maxes,
			    const std::function<bool(double t, const Vector3D &normal,
                                                     const Point2D &uv, const Vector3D &u,
                                                     const Vector3D &v)> &fn)
{
  const Vector3D ray = ray_end - eye;
  double t;

#define FACE_IMPL(FACE) \
  t = t_for_face<FACE>(eye, ray, mins, maxes); \
  if(t > 0.01 && t < numeric_limits<double>::max()) \
  { \
    const Point3D p = eye + t * ray;  \
    if(is_inside<FACE>(p, mins, maxes)) \
    { \
      Vector3D normal; \
      normal[FACE/2] = FACE % 2 == 0 ? 1 : -1; \
      Point2D uv; \
      Vector3D u, v; \
      cube_uv(FACE, p, uv, u, v); \
      if(!fn(t, normal, uv, u, v)) \
	return false; \
    } \
  }

  FACE_IMPL(0)
  FACE_IMPL(1)
  FACE_IMPL(2)
  FACE_IMPL(3)
  FACE_IMPL(4)
  FACE_IMPL(5)

  return true;
}
