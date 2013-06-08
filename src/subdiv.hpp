/*
 * Subdivide surfaces into uniform patches. Main purpose is uniform-ish photon
 * mapping, also helps with fast ray generation. This is almost trivial in the
 * case of square lights, but it's ridiculous for other kinds of lights (eg.
 * point/sphere lights and arbitrary polygons); that's why this layer of
 * abstraction is necessary.
 *
 * SurfaceSubdiv provides ways to, for each patch,
 *
 *  1. Get a list of "extreme points" in the patch. These points are sampled
 *  during caustic photon mapping in order to determine whether there are
 *  specular objects in the patch. Ideally, the implied polygon should be a
 *  minimal convex hull for the patch, but it's OK if it's actually a superset
 *  of that (as long as it's not a subset).
 *
 *  2. Generate random surface normals distributed uniformly over the surface of
 *  the patch. Obviously useful for photon mapping. Hard in general.
**/

#ifndef __SUBDIV_HPP__
#define __SUBDIV_HPP__

#include <random>
#include "algebra.hpp"

// Represents a single surface patch for some shape.
class SurfaceSubdiv
{
public:
  virtual ~SurfaceSubdiv() {}

  // Sets this object to point to the nth patch.
  virtual void set(unsigned int nsubdiv) = 0;
 
  // The extreme point list.
  virtual unsigned int num_extreme_rays() const = 0;
  virtual void extreme_ray(unsigned int n, Point3D &src, Vector3D &ray) const = 0;
  
  // Subdivisions don't necessarily have the same areas. Area sums to 1.
  virtual double area() const = 0;

  // Generate a normal vector uniformmly distributed over the light's surface.
  virtual void generate_ray(std::default_random_engine &rng, Point3D &src, Vector3D &ray) const = 0;
};

// Sphere SurfaceSubdiv implementation (spherical and point lights). Each patch
// is a triangular surface element of a subdivided isodecahedron (see
// Wikipedia's 'Geodesic grid' page).
class SphereSubdiv : public SurfaceSubdiv
{
public:
  static unsigned int num_subdivs(unsigned int nrecs);

  SphereSubdiv(const Point3D &center, unsigned int nrecs);

  virtual void set(unsigned int nsubdiv);
  virtual unsigned int num_extreme_rays() const { return 3; }
  virtual void extreme_ray(unsigned int i, Point3D &src, Vector3D &ray) const
	       { src = m_center; ray = m_points[i] - Point3D(); }
  virtual double area() const { return m_area; }
  virtual void generate_ray(std::default_random_engine &rng, Point3D &src, Vector3D &ray) const;

private:
  // Sphere center.
  Point3D m_center;

  // Number of recursive subdivisions. Each one quadruples the number of surface
  // polygons.
  const unsigned int m_nrecs;

   // Points in the current spherical triangle.
  Point3D m_points[3];

  // We generate points in a, er, partial hemisphere whose top is this point
  // (and drop the ones that don't belong in the patch).
  Point3D m_patch_center;

  // Angle away from m_center beyond which there are no patch points.
  double m_randlo;

  double m_area;

  // After generating a ray in our partial hemisphere, we clip it to three
  // planes (an infinite pyramid whose top is at the center of the sphere).
  Point3D m_plane_points[3];
  Vector3D m_plane_normals[3];

  // Rotation to move generated rays from the top of a sphere to the proper
  // patch.
  Matrix4x4 m_rotation;
};

#endif
