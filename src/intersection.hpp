/*
 * This module defines some fast intersection functions which are used in
 * several places in the code.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/


#include <limits>
#include "algebra.hpp"

enum AXIS
{
  AXIS_X=0,
  AXIS_Y,
  AXIS_Z,
};

// Represents the six faces of an axis-aligned box; positive X, negative X, etc.
enum AAFACE
{
  AAFACE_PX=0,
  AAFACE_NX,
  AAFACE_PY,
  AAFACE_NY,
  AAFACE_PZ,
  AAFACE_NZ,
};

// If we intersect the infinite plane whose normal is the given axis, offset by
// 'offset' in that directin, return the corresponding t-value.
static inline double axis_aligned_plane_intersect(const Point3D &eye, const Vector3D &ray,
				    int axis, double offset)
{
  const double at = eye[axis];
  const double dir = ray[axis];

  if(dir == 0)
    return std::numeric_limits<double>::max();

  return (offset - at) / dir;
}

// Check whether the point is within the bounds of the square corresponding to
// the specified face (AAFACE) of the axis-aligned cube given by mins, maxes.
static inline bool axis_aligned_square_contains(const Point3D &p, int face,
				const Point3D &mins, const Point3D &maxes)
{
  const int coord = face / 2;
  const int c1 = (coord + 1) % 3;
  const int c2 = (coord + 2) % 3;
  return inrange(p[c1], mins[c1], maxes[c1]) &&
	 inrange(p[c2], mins[c2], maxes[c2]);
}

// Same, but a circle rather than a square.
static inline bool axis_aligned_circle_contains(const Point3D &p, int face, double radius)
{
  const int coord = face / 2;
  const int c1 = (coord + 1) % 3;
  const int c2 = (coord + 2) % 3;
  return p[c1] * p[c1] + p[c2] * p[c2] <= radius;
}

// Check whether the axis-aligned box specified by mins, maxes is intersected by
// the specified ray (doesn't return a t-value).
static inline bool axis_aligned_box_check(const Point3D &eye, const Point3D &ray_end,
			    const Point3D &mins, const Point3D &maxes)
{
  const Vector3D ray = ray_end - eye;
  for(int i = 0; i < 6; i++)
  {
    const int axis = i / 2;
    const double offset = i % 2 == 0 ? maxes[axis] : mins[axis];
    double t = axis_aligned_plane_intersect(eye, ray, axis, offset);
    if(t < std::numeric_limits<double>::max())
    {
      const Point3D p = eye + t * ray;
      if(axis_aligned_square_contains(p, i, mins, maxes))
	return true;
    }
  }
  return false;
}
