/**
 * Code for geometric primitives.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include "algebra.hpp"
#include <cstring>

class Primitive {
public:
  typedef std::function<bool(double t, const Vector3D &normal)> IntersectFn;
  virtual bool intersect(const Point3D &eye, const Point3D &ray, const IntersectFn &fn) const = 0;
  virtual Matrix4x4 get_transform() { return Matrix4x4(); }
};

class Quadric : public Primitive
{
public:
  Quadric() { memset(this, 0, sizeof(*this)); }
  virtual bool intersect(const Point3D &eye, const Point3D &ray, const IntersectFn &fn) const;
protected:
  double A, B, C, D, E, F, G, H, J, K;
};

class Sphere : public Quadric {
public:
  Sphere() { A = D = F = 1; K = -1; };
};

class NonhierSphere : public Sphere {
public:
  NonhierSphere(const Point3D& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual Matrix4x4 get_transform();
private:
  Point3D m_pos;
  double m_radius;
};

class Cube : public Primitive {
public:
  virtual bool intersect(const Point3D &eye, const Point3D &ray, const IntersectFn &fn) const;
};

class NonhierBox : public Cube {
public:
  NonhierBox(const Point3D& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  virtual Matrix4x4 get_transform();
private:
  Point3D m_pos;
  double m_size;
};

#endif
