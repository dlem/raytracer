/**
 * Ray path tracing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
 *
 * - Ray tracer takes a lighting model, attenuation parameters?
 *    - Eg. model takes list of lights, whether each of the lights is obscured,
 *	point, ray, etc, returns 1) an ambient term (colour), 2) a transmitted
 *	term (transmitted direction is the raytracer's domain), a reflective
 *	term.
 *
 * - Shadow rays: take light, simple boolean.
 * - 
**/

#include "rt.hpp"
#include "lightingmodel.hpp"

using namespace std;

bool RayTracer::raytrace(const Point3D &src, const Point3D &ray, const RaytraceFn &fn)
{
  for(auto &geo : m_geo)
  {
    if(!geo.prim->intersect(geo.invtrans * src, geo.invtrans * ray,
	  [&geo, &fn](double t, const Vector3D &normal, const Point2D &uv,
                      const Vector3D &u, const Vector3D &v)
	  {
	    Vector3D normal_prime = geo.trans_normal * normal;
	    normal_prime.normalize();
	    return fn(geo, t, normal_prime, uv, u, v);
	  }))
    {
      return false;
    }
  }
  return true;
}

bool RayTracer::raytrace_within(const Point3D &src,
                     const Point3D &ray,
                     double tlo, double thi)
{
  return !raytrace(src , ray,
    [tlo, thi](const FlatGeo &, double tcur, const Vector3D &,
               const Point2D &, const Vector3D &, const Vector3D &)
  {
    return tcur < tlo || tcur > thi;
  });
}

double RayTracer::raytrace_min(const Point3D &src, const Point3D &ray,
    double tlo, const FlatGeo **pg,
    Vector3D &n,
    Point2D &uv,
    Vector3D &u,
    Vector3D &v)
{
  *pg = 0;
  double tmin = numeric_limits<double>::max();
  raytrace(src, ray, [tlo, &tmin, pg, &n, &uv, &u, &v]
      (const FlatGeo &g, double tcur, const Vector3D &_n,
       const Point2D &_uv, const Vector3D &_u, const Vector3D &_v)
  {
    if(tcur < tmin && tcur >= tlo)
    {
      tmin = tcur;
      *pg = &g;
      n = _n;
      uv = _uv;
      u = _u;
      v = _v;
    }
    return true;
  });
  return tmin;
}

Colour RayTracer::raytrace_recursive(const LightingModel &model,
				     const Point3D &src,
				     const Point3D &dst,
				     double acc,
				     int depth)
{
  cerr << depth << endl;
  const double threshold = 0.02;
  const double tlo = 0.1;
  const FlatGeo *g;
  Vector3D normal;
  Point2D uv;
  Vector3D u, v;
  const double t = raytrace_min(src, dst, tlo, &g, normal, uv, u, v);

  if(t >= numeric_limits<double>::max())
    return m_miss_colour(src, dst);

  Colour direct(0), transmitted(0), reflected(0);
  model.compute_lighting(*this, src, dst, t, *g, normal, uv, u, v, direct, transmitted, reflected);

  const Point3D p = src + t * (dst - src);
  const double acc_transmitted = acc * transmitted.Y();
  const double acc_reflected = acc * reflected.Y();

  Colour rv(direct);

  if(acc_reflected > threshold)
  {
    // Note that this'll cause infinite recursive for two aligned mirror
    // surfaces... need a depth limit?
    Vector3D ell = src - dst;
    ell.normalize();
    Vector3D r = 2 * ell.dot(normal) * normal - ell;
    r.normalize();
    Colour refl = raytrace_recursive(model, p, p + r, acc_reflected, depth + 1);
    rv += reflected * refl;
  }

  if(acc_transmitted > threshold)
  {
    const double ri_air = 1;
    const double ri = g->mat->ri();

    Vector3D ki = dst - src;
    ki.normalize();
    const double kidotn = ki.dot(normal);

    const double ratio = kidotn < 0 ? ri / ri_air : ri_air / ri;
    const double sinti = sqrt(1 - kidotn * kidotn);
    const double sinto = sinti / ratio;
    const double dtheta = asin(sinto) - asin(sinti);
    Vector3D ell = normal - kidotn * ki;
    ell.normalize();

    Vector3D out = cos(dtheta) * ki + sin(dtheta) * ell;

    Colour transm = raytrace_recursive(model, p, p + out, acc_transmitted, depth + 1);
    rv += transmitted * transm;
  }

  return rv;
}
