/*
 * Code for projection maps, the photon mapping preprocessing step, and
 * radiation estimates for caustics and global illumination.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __PHOTONMAP_HPP__
#define __PHOTONMAP_HPP__

#include <list>
#include <vector>
#include "rt.hpp"
#include "kdtree.hpp"
#include "cmdopts.hpp"

// Base photon map class. The idea is that you build it once using your
// geometry, then you perform radiance queries a whole bunch of times.
class PhotonMap
{
public:
  PhotonMap() {}
  virtual ~PhotonMap() {}

  // Build the photon map using the given geometry and lights.
  void build(RayTracer &rt, const std::list<Light *> &lights);
  void build_light(RayTracer &rt, const Light &light, const Colour &energy);

  // Estimates the radiance due to shot photons at a given point.
  Colour query_radiance(const Point3D &p, const Vector3D &outgoing);

  // Returns the position of the nearest photon (used for drawing the photon
  // map).
  Colour query_photon(const Point3D &pt, Vector3D &pos_rel);

  int size() const { return m_photons.size(); }

protected:
  // Number of neighbours to use in radiance estimations.
  virtual unsigned num_neighbours() = 0;

  // # of photons to shoot (if all patches are used).
  virtual unsigned num_photons() = 0;

  virtual void shoot(RayTracer &rt, const Point3D &src, const Vector3D &ray,
		     Colour energy, default_random_engine &rng) = 0;

  // Should we shoot from this patch?
  virtual bool patch_predicate(RayTracer &rt, const SurfaceSubdiv &ss) { return true; }

  // Node type used in KDTree.
  struct Photon : public KDNode
  {
    Photon(const Point3D &position, const Colour &colour, const Vector3D &outgoing)
      : KDNode(position), colour(colour), outgoing(outgoing) {}
    Colour colour;
    Vector3D outgoing;
  };

  // The KDTree.
  KDTree<Photon> m_map;

  // It's just pointers in the KD-tree; this is where the photons are actually
  // stored. Might make sense to sort it...
  std::vector<Photon> m_photons;
  std::mutex m_build_mutex;
};

// Photon map implementation for caustics.
class CausticMap : public PhotonMap
{
protected:
  virtual void shoot(RayTracer &rt, const Point3D &src, const Vector3D &ray,
		     Colour energy, default_random_engine &rng);
  virtual bool patch_predicate(RayTracer &rt, const SurfaceSubdiv &ss);
  virtual unsigned num_neighbours() { return GETOPT(caustic_num_neighbours); }
  virtual unsigned num_photons() { return GETOPT(caustic_num_photons); }
};

// Photon map implementation for global illumination.
class GIPhotonMap : public PhotonMap
{
protected:
  virtual void shoot(RayTracer &rt, const Point3D &src, const Vector3D &ray,
		     Colour energy, default_random_engine &rng);
  virtual unsigned num_neighbours() { return GETOPT(gi_num_neighbours); }
  virtual unsigned num_photons() { return GETOPT(gi_num_photons); }
};

#endif
