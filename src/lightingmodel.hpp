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
				const double t,
				const FlatGeo &geo,
				const Vector3D &normal,
				const Point2D &uv,
				const Vector3D &u,
				const Vector3D &v,
				const double refl_attn
				) const = 0;
};

class PhongModel : public LightingModel
{
public:
  PhongModel(const Colour &ambient, const std::list<Light *> &lights, CausticMap &caustics)
    : m_ambient(ambient), m_lights(lights), m_caustics(caustics)
  {}

  virtual ~PhongModel () {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Vector3D &ray,
				const double t,
				const FlatGeo &geo,
				const Vector3D &normal,
				const Point2D &uv,
				const Vector3D &u,
				const Vector3D &v,
				const double refl_attn
				) const;

private:
  const Colour &m_ambient;
  const std::list<Light *> &m_lights;
  CausticMap &m_caustics;
};
