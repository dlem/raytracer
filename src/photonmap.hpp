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

  Colour query(const Point3D &p);

protected:
  virtual void build(RayTracer &rt, const Light &light) = 0;

  struct Photon : public KDNode
  {
    Photon(const Point3D &position, const Colour &colour, const Vector3D &incident)
      : KDNode(position), colour(colour), incident(incident) {}
    Colour colour;
    Vector3D incident;
  };

  KDTree<Photon> m_map;
  std::vector<Photon> m_photons;
};

class CausticMap : public PhotonMap
{
public:
  virtual void build(RayTracer &rt, const Light &light);
private:
};

class GIPhotonMap : public PhotonMap
{
public:
  virtual void build(RayTracer &rt, const Light &light);
};

#endif
