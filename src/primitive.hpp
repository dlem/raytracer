/**
 * Code for geometric primitives.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include "algebra.hpp"
#include <cstring>

class HitInfo;

class Primitive {
public:
  typedef std::function<bool(double t, const Vector3D &normal,
                             const Point2D &uv, const Vector3D &u,
                             const Vector3D &v)> IntersectFn;
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const = 0;
  virtual void bounding_sphere(Point3D &c, double &rad) const = 0;
  virtual Matrix4x4 get_transform() { return Matrix4x4(); }
};

class Circle : public Primitive
{
public:
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const;
};

class Quadric : public Primitive
{
public:
  Quadric() { memset(this, 0, sizeof(*this)); }
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const;
  virtual bool predicate(const Point3D &pt) const { return true; }
  virtual void get_uv(const Point3D &pt, const Vector3D &normal,
		      Point2D &uv, Vector3D &u, Vector3D &v) const = 0;
protected:
  // Ax2 + Bxy + Cxz + Dy2 + Eyz + Fz2 + Gx + Hy + Jz + K;
  double A, B, C, D, E, F, G, H, J, K;
};

class Sphere : public Primitive {
public:
  Sphere() {}
  virtual void bounding_sphere(Point3D &c, double &rad) const;
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const;
};

class Cylinder : public Quadric
{
public:
  Cylinder() { A = D = 1; K = -1; }
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const; 
  virtual bool predicate(const Point3D &pt) const;
  virtual void get_uv(const Point3D &pt, const Vector3D &normal, Point2D &uv,
		      Vector3D &u, Vector3D &v) const;
  virtual void bounding_sphere(Point3D &c, double &rad) const;
  virtual Matrix4x4 get_transform();
};

class Cone : public Quadric
{
public:
  Cone() { A = D = 1; F = -1; }
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const;
  virtual bool predicate(const Point3D &pt) const;
  virtual void get_uv(const Point3D &pt, const Vector3D &normal, Point2D &uv,
		      Vector3D &u, Vector3D &v) const;
  virtual void bounding_sphere(Point3D &c, double &rad) const;
  virtual Matrix4x4 get_transform();
};

class NonhierSphere : public Sphere {
public:
  NonhierSphere(const Point3D& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual void bounding_sphere(Point3D &c, double &rad) const;
  virtual Matrix4x4 get_transform();
private:
  Point3D m_pos;
  double m_radius;
};

class Cube : public Primitive {
public:
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const;
  virtual void bounding_sphere(Point3D &c, double &rad) const;
  virtual Matrix4x4 get_transform();
};

class NonhierBox : public Cube {
public:
  NonhierBox(const Point3D& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  virtual void bounding_sphere(Point3D &c, double &rad) const;
  virtual Matrix4x4 get_transform();
private:
  Point3D m_pos;
  double m_size;
};

#endif
