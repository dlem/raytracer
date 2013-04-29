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
 *  the patch. Obviously useful for photon mapping.
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
  virtual unsigned int num_extreme_points() const = 0;
  virtual Point3D get_extreme_point(unsigned int n) const = 0;

  // Generate a normal vector uniformmly distributed over the light's surface.
  virtual Vector3D generate_ray(std::default_random_engine &rng) const = 0;
};

// Sphere SurfaceSubdiv implementation (spherical and point lights). Each patch
// is a triangular surface element of a subdivided isodecahedron (see
// Wikipedia's 'Geodesic grid' page).
class SphereSubdiv : public SurfaceSubdiv
{
public:
  static unsigned int num_subdivs(unsigned int nrecs);

  SphereSubdiv(unsigned int nrecs);

  virtual void set(unsigned int nsubdiv);
  virtual unsigned int num_extreme_points() const { return 3; }
  virtual Point3D get_extreme_point(unsigned int i) const { return m_points[i]; }
  virtual Vector3D generate_ray(std::default_random_engine &rng) const;

private:
  // Number of recursive subdivisions. Each one quadruples the number of surface
  // polygons.
  const unsigned int m_nrecs;

  // Minimum value for a random variable we use in ray generation. Depends only
  // on m_nrecs.
  const double m_randlo;

  // We generate rays wrt a fixed surface patch and then apply this
  // transformation to get rays wrt the current patch.
  Matrix4x4 m_trans;

  // Extreme points for the current surface.
  Point3D m_points[3];
};

#endif
