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

struct Box
{
  Box() {}
  void apply(const Matrix4x4 &m)
  {
    for(int i = 0; i < NUMELMS(pts); i++)
      pts[i] = m * pts[i];
  }
  Box(std::initializer_list<Point3D> list)
  {
    int i = 0;
    auto it = list.begin();
    while(i < NUMELMS(pts) && it < list.end())
      pts[i++] = *it++;
  }
  static Box centred(double size)
  {
    Box rv;
    rv.set(Point3D(-1, -1, -1), Point3D(1, 1, 1));
    return rv;
  }
  void set(const Point3D &mins, const Point3D &maxes)
  {
    pts[0] = pts[1] = pts[2] = pts[3] = mins;
    pts[1][0] = maxes[0];
    pts[2][1] = maxes[1];
    pts[3][2] = maxes[2];
    pts[4] = pts[5] = pts[6] = pts[7] = maxes;
    pts[4][0] = mins[0];
    pts[5][1] = mins[1];
    pts[6][2] = mins[2];
  }
  Point3D mins() const
  {
    Point3D rv = pts[0];
    for(int i = 1; i < NUMELMS(pts); i++)
      rv = cw_min(rv, pts[i]);
    return rv;
  }
  Point3D maxes() const
  {
    Point3D rv = pts[0];
    for(int i = 1; i < NUMELMS(pts); i++)
      rv = cw_max(rv, pts[i]);
    return rv;
  }

  Point3D pts[8];
};

static inline std::ostream &operator<<(std::ostream &os, Box &b)
{
  os << "[" << b.pts[0] << ", " << b.pts[1] << ", " << b.pts[2] << ", " << b.pts[3] << "]";
  return os;
}

class Primitive {
public:
  typedef std::function<bool(double t, const Vector3D &normal,
                             const Point2D &uv, const Vector3D &u,
                             const Vector3D &v)> IntersectFn;
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const = 0;
  virtual void bounding_box(Box &b) const = 0;
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
  virtual void bounding_box(Box &b) const;
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
  virtual void bounding_box(Box &b) const;
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
  virtual void bounding_box(Box &b) const;
  virtual Matrix4x4 get_transform();
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
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitInfo &hi) const;
  virtual void bounding_box(Box &b) const;
  virtual Matrix4x4 get_transform();
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
