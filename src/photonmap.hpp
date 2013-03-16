#ifndef __PHOTONMAP_HPP__
#define __PHOTONMAP_HPP__

#include <list>
#include <vector>
#include "rt.hpp"
#include "kdtree.hpp"

class PhotonMap
{
public:
  PhotonMap() {}
  virtual ~PhotonMap() {}

  void build(RayTracer &rt, const std::list<Light *> &lights);

  Colour query_radiance(const Point3D &p, const Vector3D &outgoing);
  Colour query_photon(const Point3D &pt, Vector3D &pos_rel);

  int size() const { return m_photons.size(); }

protected:
  virtual void build_light(RayTracer &rt, const Light &light) = 0;

  struct Photon : public KDNode
  {
    Photon(const Point3D &position, const Colour &colour, const Vector3D &outgoing)
      : KDNode(position), colour(colour), outgoing(outgoing) {}
    Colour colour;
    Vector3D outgoing;
  };

  KDTree<Photon> m_map;
  std::vector<Photon> m_photons;
};

class CausticMap : public PhotonMap
{
public:
  // Used by the --caustic-draw-pm feature.
  bool test_pm(RayTracer &rt, const Point3D &pt, const Vector3D &normal);

protected:
  virtual void build_light(RayTracer &rt, const Light &light);

};

class GIPhotonMap : public PhotonMap
{
public:
  virtual void build_light(RayTracer &rt, const Light &light);
};

#endif
