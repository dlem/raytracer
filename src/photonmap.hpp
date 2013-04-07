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

  // Estimates the radiance due to shot photons at a given point.
  Colour query_radiance(const Point3D &p, const Vector3D &outgoing);

  // Returns the position of the nearest photon (used for drawing the photon
  // map).
  Colour query_photon(const Point3D &pt, Vector3D &pos_rel);

  int size() const { return m_photons.size(); }

protected:

  // Add the photons for a light. Specific to the type of the photon map (ie,
  // caustics or GI).
  virtual void build_light(RayTracer &rt, const Light &light, const Colour &energy) = 0;

  // Number of neighbours to use in radiance estimations.
  virtual unsigned num_neighbours() = 0;

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
};

// Photon map implementation for caustics.
class CausticMap : public PhotonMap
{
public:
  // Used by the --caustic-draw-pm feature.
  bool test_pm(RayTracer &rt, const Point3D &pt, const Vector3D &normal);

protected:
  virtual void build_light(RayTracer &rt, const Light &light, const Colour &energy);
  virtual unsigned num_neighbours() { return GETOPT(caustic_num_neighbours); }

};

// Photon map implementation for global illumination.
class GIPhotonMap : public PhotonMap
{
protected:
  virtual void build_light(RayTracer &rt, const Light &light, const Colour &energy);
  virtual unsigned num_neighbours() { return GETOPT(gi_num_neighbours); }
};

#endif
