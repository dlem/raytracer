/**
 * Code for geometric primitives.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#include "primitive.hpp"
#include "mesh.hpp"

using namespace std;

bool Quadric::intersect(const Point3D &eye, const Point3D &_ray, const IntersectFn &fn) const
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
      const Vector3D normal(ddx.eval(t), ddy.eval(t), ddz.eval(t));

      // Figure out the uv.
      const Point3D pt = eye + t * ray;
      const double theta = atan(-safe_div(pt[2], pt[0]));
      const double y = pt[1];
      const Point2D uv(0.5 + theta/M_PI, 0.5 + y * 0.5);
      const Vector3D u(-pt[1], pt[0], 0);
      const Vector3D v(normal.cross(u));
      if(!fn(ts[i], normal, uv, u, v))
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

bool Cube::intersect(const Point3D &eye, const Point3D &ray, const IntersectFn &fn) const
{
  const Point3D mins(0, 0, 0);
  const Point3D maxes(1, 1, 1);
  return axis_aligned_box_check(eye, ray, mins, maxes, fn);
}

Matrix4x4 NonhierBox::get_transform()
{
  return Matrix4x4::translate(m_pos - Point3D()) *
    Matrix4x4::scale(Vector3D(m_size, m_size, m_size));
}
