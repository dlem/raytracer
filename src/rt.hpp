/**
 * Ray path tracing code.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __RT_HPP__
#define __RT_HPP__

#include <functional>
#include <list>
#include "scene.hpp"
#include "light.hpp"

#define RT_EPSILON (0.0001)

class LightingModel;

// Encapsulates information about a hit.
class HitInfo
{
public:
  HitInfo() : from(0), to(0), primary(0) {}
  double t;		    // t-value for the hit
  Vector3D normal;	    // normal pointing away from the primary geometry
  Point2D uv;		    // uv coordinates for the primary geometry
  Vector3D u, v;	    // uv vectors for the primary geometry
  const FlatGeo *from;	    // geometry the ray is exiting with the hit (or 0 if air)
  const FlatGeo *to;	    // geometry the ray is entering with the hit (or 0 if air)
  const FlatGeo *primary;   // geometry that reported the hit (never 0)

  const PhongMaterial &tomat() const { return to ? *to->mat : PhongMaterial::air; }
  const PhongMaterial &frommat() const { return from ? *from->mat : PhongMaterial::air; }
  Vector3D tonorm() const { return to == primary ? normal : -normal; }
  Vector3D fromnorm() const { return to == primary ? -normal : normal; }
};

// This is the callback type that gets passed to the main raytracing function;
// it's how the function reports hits to primitives.
typedef std::function<bool(const HitInfo &hi)> RaytraceFn;

// This class gets passed to to primitive intersection function. Primitives use
// the object's 'report' methods to report hits. If the report function returns
// false, then the primitive should stop testing and return false as well.
class HitReporter : public HitInfo
{
public:
  HitReporter(const RaytraceFn &fn) : fn(fn) {}

  inline bool report() { return fn(*this); }
  inline bool report(const HitInfo &hi)
  {
    *(HitInfo *)this = hi;
    return report();
  }
  inline bool report(double _t, const Vector3D &_normal, const Point2D &_uv,
	      const Vector3D &_u, const Vector3D &_v)
  {
    t = _t;
    normal = _normal;
    uv = _uv;
    u = _u;
    v = _v;
    return report();
  }

private:
  const RaytraceFn &fn;
};

// Type of the miss function -- the function that gets called to determine the
// colour when we fail to hit an object in the scene. It can make use of the
// point and vector to draw an intersesting background.
typedef std::function<Colour(const Point3D &, const Vector3D &)> MissColourFn;

// Possible actions to take in the Russian roulette ray tracing function.
enum RT_ACTION
{
  RT_DIFFUSE=0,	  // Diffuse reflection.
  RT_REFLECT,	  // Specular reflection (Fresnel eqns => reflection or transmission)
  RT_TRANSMIT,
  RT_ABSORB,	  // The light gets absorbed by the material.
  RT_ACTION_COUNT
};

// Function type used as an argument to the Russian roulette ray tracing
// function. It gets called with every hit, and must return the next action to
// take.
typedef std::function<bool(const Point3D &p,     // the hit point
			   const Vector3D &incident,  // the incident vector
			   const Colour &photon,      // the colour of the photon _after_ it hits
			   RT_ACTION action	      // the probababilities for each action, indexed by the RT_ACTION enums
			   )> RussianFn;


///////////////////////////////////////////////////////////////////////////////
// Exposes a bunch of raytracing methods for different purposes.
class RayTracer
{
public:
  // Takes geometry to be used as well as a function to govern the colour when
  // we miss hitting anything.
  RayTracer(const FlatList &geo, const MissColourFn &miss_colour)
    : m_geo(geo)
    , m_miss_colour(miss_colour)
  {}

  // Defaults to the miss colour from CmdOpts.
  RayTracer(const FlatList &geo);

  // Does recursive raytracing (ie, shoots secondary rays in specular reflection
  // directions) and returns the resulting colour.
  Colour raytrace_recursive(const LightingModel &model, // used to determine the colour of surfaces
			    const Point3D &src,
			    const Vector3D &ray,
			    double *dist = 0,		// if non-null, gets set to distance to the first hit
			    double acc = 1,		// accumulated luminance attenuation
			    int depth = 0);

  // Russian roulette raytracing algorithm.
  void raytrace_russian(const Point3D &src,
			const Vector3D &ray,
			const Colour &acc,		// the accumulated colour
			std::default_random_engine &rng,
			const RussianFn &fn,		// determines next action to take, processes hits	
			int depth = 0);


  // Returns true if any primitive hit yields a t-value in [tlo, thi].
  bool raytrace_within(const Point3D &src,
		       const Vector3D &ray,
		       double tlo, double thi);

  // Finds the hit with the smallest t-value greater or equal to tlo and returns
  // the relevant information.
  bool raytrace_min(const Point3D &src,
		    const Vector3D &ray,
		    double tlo, HitInfo &hi);

  const FlatList &geo() { return m_geo; }

private:
  // The function for tracing a single ray. Finds all of the intersections with
  // objects in the scene and reports them through 'fn.' Stops if 'fn' returns
  // false.
  bool raytrace(const Point3D &src, const Vector3D &ray, const RaytraceFn &fn);

  const FlatList &m_geo;
  const MissColourFn m_miss_colour;
};

// Generates a random ray.
template<typename RNG>
static inline Vector3D generate_ray(RNG &rng)
{
  return Vector3D(-1 + rng() * (2. / rng.max()),
		  -1 + rng() * (2. / rng.max()),
		  -1 + rng() * (2. / rng.max()));
}

#endif
