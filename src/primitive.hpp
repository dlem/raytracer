/**
 * Code for geometric primitives.
 *
 * Donated code.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include "algebra.hpp"
#include <cstring>

class HitReporter;

// Represents a bounding box. The methods are meant to make it really easy to
// bring this bounding box up through a hierarchy.
struct Box
{
  Box() {}
  // Apply a transform to the bounding box.
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
  // Create a box at (0, 0, 0) with size 'size'.
  static Box centred(double size)
  {
    Box rv;
    rv.set(Point3D(-1, -1, -1), Point3D(1, 1, 1));
    return rv;
  }
  // Set the bounding box according to the specified axis-aligned box.
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
  // Have the primitive report intersections of the ray with itself.
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const = 0;

  // Get a bounding box for the primitive.
  virtual void bounding_box(Box &b) const = 0;

  // Determine whether it's a CSG primitive. This replaces a surprisingly
  // expensive dynamic cast.
  virtual bool is_csg() const { return false; }

  // Transforms to be applied to a primitive automatically. Used for
  // nonhierachal primitives and to make my life easier with cones and
  // cylinders.
  virtual Matrix4x4 get_transform() { return Matrix4x4(); }
};

// Quadric primitive. The UV's must be implemented manually by overriding
// get_uv. Note that the compiler won't be able to optimize this nearly as well
// as a special-case class.
class Quadric : public Primitive
{
public:
  Quadric() { memset(this, 0, sizeof(*this)); }
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const;
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
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const;
};

class Cylinder : public Quadric
{
public:
  Cylinder() { A = D = 1; K = -1; }
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const; 
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
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const;
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
  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const;
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
