#include <functional>
#include <list>
#include "light.hpp"
#include "algebra.hpp"
#include "scene.hpp"

class RayTracer;

class LightingModel
{
public:
  virtual ~LightingModel() {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Point3D &ray,
				const double t,
				const FlatGeo &geo,
				const Vector3D &normal,
				const Point2D &uv,
				const Vector3D &u,
				const Vector3D &v
				) const = 0;
};

class PhongModel : public LightingModel
{
public:
  PhongModel(const Colour &ambient, const std::list<Light *> &lights)
    : m_ambient(ambient), m_lights(lights)
  {}

  virtual ~PhongModel () {}

  virtual Colour compute_lighting(RayTracer &rt,
				const Point3D &src,
				const Point3D &ray,
				const double t,
				const FlatGeo &geo,
				const Vector3D &normal,
				const Point2D &uv,
				const Vector3D &u,
				const Vector3D &v
				) const;

private:
  const Colour &m_ambient;
  const std::list<Light *> &m_lights;
};
