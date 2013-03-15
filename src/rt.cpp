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

#if 0
static bool raytrace_common(const LightingModel &model,
			    const Point3D &src,
			    const Point3D &dst,
			    const FlatGeo *&g,
			    Colour &ctransmitted,
			    Colour &creflected,
			    Vector3D &normal,
			    Point2D &uv,
			    Vector3D &u,
			    Vector3D &v,
			    Vector3D &ray_reflected,
			    Vector3D &ray_transmitted,
			    )
{
  const double tlo = 0.1;
  const double t = raytrace_min(src, dst, tlo, &g, normal, uv, u, v);

  if(t >= numeric_limits<double>::max())
    return false;

  const double ri_air = 1;
  const double ri = g->mat->ri();
  const double opacity = g->mat->opacity();

  double direct, transmitted, reflected;
  Vector3D ray = dst - src;
  ray.normalize();
  const bool penetrating = ray.dot(normal) < 0;
  const Vector3D n = penetrating ? normal : -normal;
  ray_reflected = ray - 2 * ray.dot(n) * n;

  if(ri == 0)
  {
    // If this material has an ri of zero, just use phong calculations, as it
    // has no reflection or refraction.
    direct = reflected = 1;
    transmitted = 0;
  }
  else
  {
    const double n1 = penetrating ? ri_air : ri;
    const double n2 = penetrating ? ri : ri_air;
    const double cosi = ray.dot(-n);
    const double sini = sqrt(1 - cosi * cosi);
    const double sint = n1 / n2 * sini;
    double r;

    if(sint >= 1)
    {
      // Total internal reflection.
      r = 1;
    }
    else
    {
      const double cost = sqrt(1 - sint * sint);
      r = fresnel(n1, n2, cosi, cost);
      // Calculate the rafracted ray.
      ray_transmitted = n1/n2 * ray + (n1/n2 * cosi - cost) * n;
    }

    direct = opacity;
    transmitted = (1 - r) * (1 - opacity);
    reflected = r;
  }

  ctransmitted = transmitted * g->mat->ks(uv);
  creflected = reflected * g->mat->ks(uv);
}
#endif

Colour RayTracer::raytrace_recursive(const LightingModel &model,
				     const Point3D &src,
				     const Point3D &dst,
				     double acc,
				     int depth)
{
  const double threshold = 0.02;
  const double tlo = 0.1;
  const FlatGeo *g;
  Vector3D normal;
  Point2D uv;
  Vector3D u, v;
  const double t = raytrace_min(src, dst, tlo, &g, normal, uv, u, v);

  if(t >= numeric_limits<double>::max())
    return m_miss_colour(src, dst);

  if(depth > 12)
  {
    // This can happen with in certain situations (eg. perfect mirrors or
    // repeated total internal reflection on the _inside_ of certain
    // primitives). The best thing we can do in this situations is just
    // calculate the phong lighting.
    return model.compute_lighting(*this, src, dst, t, *g, normal, uv, u, v);
  }

  const double ri_air = 1;
  const double ri = g->mat->ri();
  const double opacity = g->mat->opacity();

  double direct, transmitted, reflected;
  Vector3D ray = dst - src;
  ray.normalize();
  const bool penetrating = ray.dot(normal) < 0;
  const Vector3D n = penetrating ? normal : -normal;
  const Vector3D ray_reflected = ray - 2 * ray.dot(n) * n;
  Vector3D ray_transmitted;

  if(ri == 0)
  {
    // If this material has an ri of zero, just use phong calculations, as it
    // has no reflection or refraction.
    direct = reflected = 1;
    transmitted = 0;
  }
  else
  {
    const double n1 = penetrating ? ri_air : ri;
    const double n2 = penetrating ? ri : ri_air;
    const double cosi = ray.dot(-n);
    const double sini = sqrt(1 - cosi * cosi);
    const double sint = n1 / n2 * sini;
    double r;

    if(sint >= 1)
    {
      // Total internal reflection.
      r = 1;
    }
    else
    {
      const double cost = sqrt(1 - sint * sint);
      r = fresnel(n1, n2, cosi, cost);
      // Calculate the rafracted ray.
      ray_transmitted = n1/n2 * ray + (n1/n2 * cosi - cost) * n;
    }

    direct = opacity;
    transmitted = (1 - r) * (1 - opacity);
    reflected = r;
  }

  const Point3D p = src + t * (dst - src);
  Colour ctransmitted = transmitted * g->mat->ks(uv);
  Colour creflected = reflected * g->mat->ks(uv);
  const double acc_transmitted = acc * ctransmitted.Y();
  const double acc_reflected = acc * creflected.Y();

  Colour rv(0);

  Colour cdirect = model.compute_lighting(*this, src, dst, t, *g, normal, uv, u, v);
  rv += direct * cdirect;

  if(acc_reflected > threshold)
  {
    const Point3D dst_refl = p + ray_reflected;
    creflected = creflected * raytrace_recursive(model, p, dst_refl, acc_reflected, depth + 1);
    rv += creflected;
  }

  if(acc_transmitted > threshold)
  {
    const Point3D dst_trans = p + ray_transmitted;
    Colour ctransmitted = raytrace_recursive(model, p, dst_trans, acc_transmitted, depth + 1);
    rv += transmitted * ctransmitted;
  }

  return rv;
}
