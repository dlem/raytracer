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

double fresnel(double n1, double n2, double cosi, double cost)
{
  // degreve -- reflection -- refraction pdf
  double rperp = (n1 * cosi - n2 * cost) / (n1 * cosi + n2 * cost);
  rperp *= rperp;
  double rpar = (n1 * cost - n2 * cosi) / (n1 * cost + n2 * cosi);
  rpar *= rpar;
  return (rpar + rperp) / 2;
}

// Returns the proportion of specular power that should go to reflection. The
// rest should go to refraction.
double compute_specular(const Vector3D &incident,     // Must be unit.
			const FlatGeo &geo,
			const Vector3D &_n,	      // The normal.
			Vector3D &ray_reflected,
			Vector3D &ray_transmitted)
{
  const double ri_air = 1;
  const double ri = geo.mat->ri();
  const bool penetrating = incident.dot(_n) < 0;
  const Vector3D n = penetrating ? _n : -_n;
  ray_reflected = incident - 2 * incident.dot(n) * n;

  if(ri >= numeric_limits<double>::max())
    // Only reflective.
    return 1;

  const double n1 = penetrating ? ri_air : ri;
  const double n2 = penetrating ? ri : ri_air;
  const double cosi = incident.dot(-n);
  const double sini = sqrt(1 - cosi * cosi);
  const double sint = n1 / n2 * sini;

  if(sint >= 1)
    // Total internal reflection.
    return 1;

  const double cost = sqrt(1 - sint * sint);
  ray_transmitted = n1/n2 * incident + (n1/n2 * cosi - cost) * n;
  return fresnel(n1, n2, cosi, cost);
}

Colour RayTracer::raytrace_recursive(const LightingModel &model,
				     const Point3D &src,
				     const Point3D &dst,
				     double acc,
				     int depth)
{
  const double threshold = 0.02;
  const double tlo = 0.1;
  const FlatGeo *g;
  Vector3D normal, u, v;
  Point2D uv;

  const double t = raytrace_min(src, dst, tlo, &g, normal, uv, u, v); 

  if(t >= numeric_limits<double>::max())
    return m_miss_colour(src, dst);

  if(depth > 12)
  {
    // This can happen in certain situations (eg. perfect mirrors or
    // repeated total internal reflection on the _inside_ of certain
    // primitives). The best thing we can do in this situations is just
    // calculate the phong lighting.
    return model.compute_lighting(*this, src, dst, t, *g, normal, uv, u, v, 1);
  }

  Vector3D incident = dst - src;
  const Point3D p = src + t * incident;
  // Do we need this normalize, or is it already unit?
  incident.normalize();
  Vector3D ray_reflected, ray_transmitted;
  const double r = compute_specular(incident, *g, normal, ray_reflected, ray_transmitted);

  Colour rv(0);

  Colour cdirect = model.compute_lighting(*this, src, dst, t, *g, normal, uv, u, v, r);
  rv += cdirect;

  const Colour cspecular = g->mat->ks(uv);
  const Colour creflected = r * cspecular;
  const Colour ctransmitted = (1 - r) * cspecular;
  const double acc_reflected = acc * creflected.Y();
  const double acc_transmitted = acc * ctransmitted.Y();

  if(acc_reflected > threshold)
  {
    const Point3D dst_refl = p + ray_reflected;
    rv += creflected * raytrace_recursive(model, p, dst_refl, acc_reflected, depth + 1);
  }

  if(acc_transmitted > threshold)
  {
    const Point3D dst_trans = p + ray_transmitted;
    rv += ctransmitted * raytrace_recursive(model, p, dst_trans, acc_transmitted, depth + 1);
  }

  return rv;
}

void RayTracer::raytrace_russian(const Point3D &src,
		      const Point3D &dst, const Colour &acc,
		      const RussianFn &fn, int depth)
{
  const double tlo = 0.1;
  const FlatGeo *g;
  Vector3D normal, u, v;
  Point2D uv;

  const double t = raytrace_min(src, dst, tlo, &g, normal, uv, u, v);

  if(t >= numeric_limits<double>::max())
    return;

  if(depth > 12)
    // We've no use for information about photons hitting spec surfaces, so...
    return;

  Vector3D incident = dst - src;
  const Point3D p = src + t * incident;
  // Do we need this, or is it already unit?
  incident.normalize();
  const Colour cdiffuse = acc * g->mat->kd(uv);
  const Colour cspecular = acc * g->mat->ks(uv);

  double prs[RT_ACTION_COUNT];
  prs[RT_DIFFUSE] = cdiffuse.Y();
  prs[RT_SPECULAR] = cspecular.Y();

  for(int i = 1; i < RT_ACTION_COUNT - 1; i++)
    prs[i] += prs[i - 1];

  static_assert(RT_ABSORB == RT_ACTION_COUNT - 1, "");
  prs[RT_ABSORB] = 1;

  const RT_ACTION action = fn(p, incident, cdiffuse, prs);

  if(action == RT_ABSORB)
    return;

  if(action == RT_SPECULAR)
  {
    Vector3D ray_reflected;
    Vector3D ray_transmitted;
    const double r = compute_specular(incident, *g, normal, ray_reflected, ray_transmitted);
    if((rand() % 101) * 0.01 > r)
    {
      // Refraction.
      raytrace_russian(p, p + ray_transmitted, (1 - r) * cspecular, fn, depth + 1);
    }
    else
      // Reflection.
      raytrace_russian(p, p + ray_reflected, r * cspecular, fn, depth + 1);
  }
  else
  {
    assert(action == RT_DIFFUSE);
    // Only required for GI.
    assert(0 && "Not implemented!");
  }
}
