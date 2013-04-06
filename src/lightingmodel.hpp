/*
 * This module defines the LightingModel interface as well as several
 * implementations.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#include <functional>
#include <list>
#include "light.hpp"
#include "algebra.hpp"
#include "scene.hpp"
#include "photonmap.hpp"

class RayTracer;

class LightingModel
{
public:
  virtual ~LightingModel() {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Vector3D &ray,
				const HitInfo &hi,
				const double refl_attn
				) const = 0;
};

class PhongModel : public LightingModel
{
public:
  PhongModel(const Colour &ambient, const std::list<Light *> &lights, CausticMap &caustics,
	     GIPhotonMap *gimap = 0)
    : m_ambient(ambient), m_lights(lights), m_caustics(caustics), m_gimap(gimap)
  {}

  virtual ~PhongModel () {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Vector3D &ray,
				const HitInfo &hi,
				const double refl_attn
				) const;

private:
  const Colour &m_ambient;
  const std::list<Light *> &m_lights;
  CausticMap &m_caustics;
  GIPhotonMap *m_gimap;
};

// This lighting model draws colour only at points which are sufficiently close
// to photons. Good visualization of photon map.
class PhotonDrawModel : public LightingModel
{
public:
  PhotonDrawModel(PhotonMap &map)
    : m_map(map)
  {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Vector3D &ray,
				const HitInfo &hi,
				const double refl_attn
				) const;

private:
  PhotonMap &m_map;
};

// This lighting model draws caustics and nothing else.
class PhotonsOnlyModel : public LightingModel
{
public:
  PhotonsOnlyModel(PhotonMap &map)
    : m_map(map)
  {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Vector3D &ray,
				const HitInfo &hi,
				const double refl_attn
				) const;

private:
  PhotonMap &m_map;
};
