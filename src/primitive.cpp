/**
 * Code for geometric primitives.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#include "primitive.hpp"
#include <iostream>
#include "mesh.hpp"
#include "intersection.hpp"
#include "rt.hpp"

using namespace std;

bool Quadric::intersect(const Point3D &eye, const Point3D &_ray, HitReporter &hr) const
{
  Polynomial<2> x, y, z;
  const Vector3D ray = _ray - eye;
  x[0] = eye[0];
  x[1] = ray[0];
  y[0] = eye[1];
  y[1] = ray[1];
  z[0] = eye[2];
  z[1] = ray[2];

  const Polynomial<2> eqn = A * x * x + B * x * y + C * x * z
    + D * y * y + E * y * z + F * z * z + G * x + H * y + J * z + K;

  double ts[2];
  auto nhits = eqn.solve(ts);

  if(nhits > 0)
  {
    const Polynomial<2> ddx = 2 * A * x + B * y + C * z + G,
			ddy = 2 * D * y + B * x + E * z + H,
			ddz = 2 * F * z + C * x + E * y + J;

    for(int i = 0; i < nhits; i++)
    {
      // Figure out the normal.
      const double t = ts[i];
      const Point3D pt = eye + t * ray;
      if(!predicate(pt))
	continue;

      Vector3D normal(ddx.eval(t), ddy.eval(t), ddz.eval(t));

      // Figure out the uv.
      Point2D uv;
      Vector3D u, v;
      get_uv(pt, normal, uv, u, v);

      if(!hr.report(ts[i], normal, uv, u, v))
	return false;
    }
  }

  return true;
}

// This is basically the same as Quadric::intersect, but Quadric::intersect is
// too general to be fast, so we're reimplementing it here.
bool Sphere::intersect(const Point3D &eye, const Point3D &dst, HitReporter &hr) const
{
  Polynomial<2> x, y, z;
  const Vector3D ray = dst - eye;
  x[0] = eye[0];
  x[1] = ray[0];
  y[0] = eye[1];
  y[1] = ray[1];
  z[0] = eye[2];
  z[1] = ray[2];
  
  const Polynomial<2> eqn = x * x + y * y + z * z + (-1);

  double ts[2];
  auto nhits = eqn.solve(ts);

  if(nhits > 0)
  {
    for(int i = 0; i < nhits; i++)
    {
      const double t = ts[i];
      const Point3D pt = eye + t * ray;

      // The normal is just the intersection point in the sphere's coordinate
      // system. We can avoid the normalize since this is a unit sphere.
      Vector3D normal(pt - Point3D());

      // Figure out the uv.
      const double theta = atan2(-pt[2], pt[0]);
      const double y = pt[1];
      const Point2D uv(0.5 + theta/M_PI, 0.5 + y * 0.5);
      Vector3D u = Vector3D(pt[1], -pt[0], 0);
      u.normalize();
      const Vector3D v = normal.cross(u);

      if(!hr.report(ts[i], normal, uv, u, v))
	return false;
    }
  }

  return true;
}

Matrix4x4 NonhierSphere::get_transform()
{
  return Matrix4x4::translate(m_pos - Point3D()) *
    Matrix4x4::scale(Vector3D(m_radius, m_radius, m_radius));
}

void Sphere::bounding_box(Box &b) const
{
  b = Box::centred(1);
}

void cube_uv(int facenum, const Point3D &p, Point2D &uv, Vector3D &u, Vector3D &v)
{
  double cu = 1/4.;
  double cv = 1/3.;

  static const struct
  {
    Point3D base;
    Vector3D u, v;
    int facex, facey;
    int cu;
    double ucoef;
    int cv;
    double vcoef;
  } faces[] = {
    { {1, 1, 1}, {0, -1, 0}, {0, 0, -1}, 2, 1, 1, -1, 2, -1 },
    { {0, 0, 1}, {0, 1, 0}, {0, 0, -1}, 0, 1, 1, 1, 2, -1 },
    { {0, 1, 1}, {1, 0, 0}, {0, 0, -1}, 1, 1, 0, 1, 2, -1 },
    { {1, 0, 1}, {-1, 0, 0}, {0, 0, -1}, 3, 1, 0, -1, 2, -1 },
    { {0, 0, 1}, {1, 0, 0}, {0, 1, 0}, 1, 0, 0, 1, 1, 1 },
    { {0, 1, 0}, {1, 0, 0}, {0, -1, 0}, 1, 2, 0, 1, 1, -1 },
  };

  auto &face = faces[facenum];
  const Vector3D delta = p - face.base;
  uv = Point2D(cu * (face.facex + face.ucoef * delta[face.cu]),
	       cv * (face.facey + face.vcoef * delta[face.cv]));

  u = face.u;
  v = face.v;
}

Matrix4x4 Cube::get_transform()
{
  return Matrix4x4::scale(Vector3D(2, 2, 2)) * Matrix4x4::translate(Vector3D(-0.5, -0.5, -0.5));
}

bool Cube::intersect(const Point3D &eye, const Point3D &ray_end, HitReporter &hr) const
{
  const Vector3D ray = ray_end - eye;
  const Point3D mins(0, 0, 0);
  const Point3D maxes(1, 1, 1);

  for(int i = 0; i < 6; i++)
  {
    const int axis = i / 2;
    const double offset = i % 2 == 0 ? 1 : 0;
    const double t = axis_aligned_plane_intersect(eye, ray, i/2, offset);
    if(t < numeric_limits<double>::max())
    {
      const Point3D p = eye + t * ray;
      if(axis_aligned_square_contains(p, i, mins, maxes))
      {
	Vector3D normal;
	normal[axis] = i % 2 == 0 ? 1 : -1;
	Point2D uv;
	Vector3D u, v;
	cube_uv(i, p, uv, u, v);
	if(!hr.report(t, normal, uv, u, v))
	  return false;
      }
    }
  }
  return true;
}

void Cube::bounding_box(Box &b) const
{
  b = Box::centred(1);
}


Matrix4x4 NonhierBox::get_transform()
{
  return Matrix4x4::translate(m_pos - Point3D()) *
    Matrix4x4::scale(Vector3D(m_size, m_size, m_size));
}

bool Cylinder::intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const
{
  if(!Quadric::intersect(eye, ray, hr))
    return false;

  const Vector3D _ray = ray - eye;
  const int circles[] = {AAFACE_PZ, AAFACE_NZ};
  const double offsets[] = {1., -1.};

  for(int i = 0; i < NUMELMS(circles); i++)
  {
    const int face = circles[i];
    const int axis = face / 2;
    const double offset = offsets[i];
    const double t = axis_aligned_plane_intersect(eye, _ray, axis, offset);
    if(t < numeric_limits<double>::max())
    {
      const Point3D p = eye + t * _ray;
      if(axis_aligned_circle_contains(p, face, 1.))
      {
	Vector3D normal;
	normal[axis] = offset;
	const double _u = 0.25 * ((face == AAFACE_PZ ? 1 : 3) + p[0]);
	const double _v = 0.25 * (3 + p[1]);
	Point2D uv(_u, _v);
	Vector3D u(1, 0, 0);
	Vector3D v(0, 1, 0);
	if(!hr.report(t, normal, uv, u, v))
	  return false;
      }
    }
  }

  return true;
}

void Cylinder::bounding_box(Box &b) const
{
  b = Box::centred(1);
}

bool Cylinder::predicate(const Point3D &pt) const
{
  return inrange(pt[2], -1., 1.);
}

void Cylinder::get_uv(const Point3D &pt, const Vector3D &normal,
		      Point2D &uv, Vector3D &u, Vector3D &v) const
{
  // This function handles uvs on the non-planar part.
  // Find theta, translate it into a coordinate from 0 to 1.
  const double theta = atan2(pt[1], pt[0]);
  const double z = pt[2];
  uv[0] = 0.5 + theta / 2 / M_PI;
  uv[1] = 0.25 + z * 0.25;
  v = Vector3D(0, 0, 1);
  u = v.cross(normal);
  assert(approx(v.dot(normal), 0.));
  assert(approx(u.dot(normal), 0.));
}

Matrix4x4 Cylinder::get_transform()
{
#if 1
  return Matrix4x4::rotate('x', 90);
#else
  return Matrix4x4();
#endif
}

bool Cone::intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const
{
  if(!Quadric::intersect(eye, ray, hr))
    return false;

  const Vector3D _ray = ray - eye;
  const int circles[] = {AAFACE_PZ};
  const double offsets[] = {1.};

  for(int i = 0; i < NUMELMS(circles); i++)
  {
    const int face = circles[i];
    const int axis = face / 2;
    const double offset = offsets[i];
    const double t = axis_aligned_plane_intersect(eye, _ray, axis, offset);
    if(t < numeric_limits<double>::max())
    {
      const Point3D p = eye + t * _ray;
      if(axis_aligned_circle_contains(p, face, 1.))
      {
	Vector3D normal;
	normal[axis] = offset;
	const double _u = 0.5 * (1 + p[0]);
	const double _v = 0.25 * (3 + p[1]);
	Point2D uv(_u, _v);
	Vector3D u(1, 0, 0);
	Vector3D v(0, 1, 0);
	if(!hr.report(t, normal, uv, u, v))
	  return false;
      }
    }
  }

  return true;
}

void Cone::bounding_box(Box &b) const
{
  b = Box({Point3D(-1, -1, -1), Point3D(1, -1, -1),
	   Point3D(-1, 1, -1), Point3D(-1, -1, 1)});
}

bool Cone::predicate(const Point3D &pt) const
{
  return inrange(pt[2], 0., 1.);
}

void Cone::get_uv(const Point3D &pt, const Vector3D &normal,
		  Point2D &uv, Vector3D &u, Vector3D &v) const
{
  const double theta = atan2(-pt[2], pt[0]);
  const double y = pt[1];
  uv[0] = 0.5 + theta / M_PI;
  uv[1] = 0.25 + y * 0.25;
  u = Vector3D(pt[1], -pt[0], 0);
  u.normalize();
  v = normal.cross(u);
}

// Translate it so that it's centered at 0.
Matrix4x4 Cone::get_transform()
{
  return Matrix4x4::rotate('x', 90) * Matrix4x4::translate(Vector3D(0, 0, -0.5));
}
