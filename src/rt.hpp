/**
 * Ray path tracing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#ifndef __RT_HPP__
#define __RT_HPP__

#include <functional>
#include <list>
#include "scene.hpp"
#include "light.hpp"

class LightingModel;

// Callback type that gets passed to my main raytracing function.
typedef std::function<bool(const FlatGeo &geo, double t,
                           const Vector3D &normal,
                           const Point2D &uv,
                           const Vector3D &u,
                           const Vector3D &v)>
        RaytraceFn;


typedef std::function<Colour(const Point3D &, const Point3D &)> MissColourFn;



// Stores ray tracing "context" -- ie, anything I might need and don't want to
// pass around in a massive argument list.
class RayTracer
{
public:
  RayTracer(const FlatList &geo, const MissColourFn &miss_colour)
    : m_geo(geo)
    , m_miss_colour(miss_colour)
  {}

  Colour raytrace_recursive(const LightingModel &model, const Point3D &src,
			    const Point3D &ray, double acc = 1, int depth = 0);

  // Returns true if any primitive hit yields a t-value in [tlo, thi].
  bool raytrace_within(const Point3D &src,
		       const Point3D &ray,
		       double tlo, double thi);

  // Finds the hit with the smallest t-value greater or equal to tlo and returns
  // the relevant information.
  double raytrace_min(const Point3D &src,
		      const Point3D &ray,
		      double tlo,
		      const FlatGeo **pg,
		      Vector3D &normal,
		      Point2D &uv,
		      Vector3D &u,
		      Vector3D &v);

private:
  bool raytrace(const Point3D &src, const Point3D &ray, const RaytraceFn &fn);

  const FlatList &m_geo;
  const MissColourFn m_miss_colour;
};

#endif
