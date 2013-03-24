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
**/

#include "rt.hpp"
#include "lightingmodel.hpp"
#include "cmdopts.hpp"

using namespace std;

bool RayTracer::raytrace(const Point3D &src, const Vector3D &ray, const RaytraceFn &fn)
{
  const Point3D dst = src + ray;

  HitInfo hi(fn);
  hi.med = &PhongMaterial::air;

  for(auto &geo : m_geo)
  {
    hi.geo = &geo;
    if(!geo.prim->intersect(geo.invtrans * src, geo.invtrans * dst, hi))
      return false;
  }

  return true;
}

bool RayTracer::raytrace_within(const Point3D &src,
                     const Vector3D &ray,
                     double tlo, double thi)
{
  return !raytrace(src , ray,
    [tlo, thi](const FlatGeo &g, const Material &, double tcur, const Vector3D &,
               const Point2D &, const Vector3D &, const Vector3D &)
  {
    return tcur < tlo || tcur > thi;
  });
}

double RayTracer::raytrace_min(const Point3D &src, const Vector3D &ray,
    double tlo, const FlatGeo **pg, const Material **med,
    Vector3D &n,
    Point2D &uv,
    Vector3D &u,
    Vector3D &v)
{
  *pg = 0;
  double tmin = numeric_limits<double>::max();
  raytrace(src, ray, [tlo, &tmin, pg, &n, &uv, &u, &v, med]
      (const FlatGeo &g, const Material &_med, double tcur, const Vector3D &_n,
       const Point2D &_uv, const Vector3D &_u, const Vector3D &_v)
  {
    if(tcur < tmin && tcur >= tlo)
    {
      tmin = tcur;
      *pg = &g;
      *med = &_med;
      n = _n;
      uv = _uv;
      u = _u;
      v = _v;
    }
    return true;
  });
  if(*pg)
  {
    n = (*pg)->trans_normal * n;
    n.normalize();
  }
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
			const Material &obj,
			const Material &medium,
			const Vector3D &_n,	      // The normal.
			Vector3D &ray_reflected,
			Vector3D &ray_transmitted)
{
  const double ri_medium = medium.ri();
  const double ri_obj = obj.ri();
  const bool penetrating = incident.dot(_n) < 0;
  const Vector3D n = penetrating ? _n : -_n;
  ray_reflected = incident - 2 * incident.dot(n) * n;
  ray_reflected.normalize();

  if(ri_obj >= numeric_limits<double>::max())
    // Only reflective. This applies to both sides, by edict.
    return 1;

  const double n1 = penetrating ? ri_medium : ri_obj;
  const double n2 = penetrating ? ri_obj : ri_medium;
  const double cosi = incident.dot(-n);
  const double sini = sqrt(1 - cosi * cosi);
  const double sint = n1 / n2 * sini;

  if(sint >= 1)
    // Total internal reflection.
    return 1;

  const double cost = sqrt(1 - sint * sint);
  ray_transmitted = n1/n2 * incident + (n1/n2 * cosi - cost) * n;
  ray_transmitted.normalize();
  return fresnel(n1, n2, cosi, cost);
}

Colour RayTracer::raytrace_recursive(const LightingModel &model,
				     const Point3D &src,
				     const Vector3D &incident,
				     double acc,
				     int depth)
{
  const double threshold = 0.02;
  const FlatGeo *g;
  const Material *medium;
  Vector3D normal, u, v;
  Point2D uv;

  const double t = raytrace_min(src, incident, RT_EPSILON, &g, &medium, normal, uv, u, v); 

  if(t >= numeric_limits<double>::max())
    return m_miss_colour(src, incident);

  const bool penetrating = incident.dot(normal) < 0;
  const Material &mat = penetrating ? *g->mat : *medium;

  if(depth > 12)
  {
    // This can happen in certain situations (eg. perfect mirrors or
    // repeated total internal reflection on the _inside_ of certain
    // primitives). The best thing we can do in this situations is just
    // calculate the phong lighting.
    return model.compute_lighting(*this, src, incident, t, *g, mat, normal, uv, u, v, 1);
  }

  const Point3D p = src + t * incident;
  Vector3D ray_reflected, ray_transmitted;
  const double r = compute_specular(incident, *g->mat, *medium, normal, ray_reflected, ray_transmitted);

  Colour rv(0);

  Colour cdirect = model.compute_lighting(*this, src, incident, t, *g, mat, normal, uv, u, v, r);
  rv += cdirect;

  const Colour cspecular = mat.ks(uv);
  const Colour creflected = r * cspecular;
  const Colour ctransmitted = (1 - r) * cspecular;
  const double acc_reflected = acc * creflected.Y();
  const double acc_transmitted = acc * ctransmitted.Y();

  if(acc_reflected > threshold)
  {
    rv += creflected * raytrace_recursive(model, p, ray_reflected, acc_reflected, depth + 1);
  }

  if(acc_transmitted > threshold)
  {
    rv += ctransmitted * raytrace_recursive(model, p, ray_transmitted, acc_transmitted, depth + 1);
  }

  return rv;
}

void RayTracer::raytrace_russian(const Point3D &src,
		      const Vector3D &incident, const Colour &acc,
		      const RussianFn &fn, int depth)
{
  const FlatGeo *g;
  const PhongMaterial *medium;
  Vector3D normal, u, v;
  Point2D uv;

  const double t = raytrace_min(src, incident, RT_EPSILON, &g, &medium, normal, uv, u, v);

  if(t >= numeric_limits<double>::max())
    return;

  if(depth > 12)
    // We've no use for information about photons hitting spec surfaces, so...
    return;

  const bool penetrating = incident.dot(normal) < 0;

  Vector3D ray_reflected;
  Vector3D ray_transmitted;
  const double r = compute_specular(incident, *g->mat, *medium, normal,
				    ray_reflected, ray_transmitted);

  const Material &mat = penetrating ? *g->mat : *medium;
  const Point3D p = src + t * incident;
  // Giving incorrect uv coordinates to a medium won't hurt...
  const Colour cdiffuse = mat.kd(uv);
  const Colour cspecular = mat.ks(uv);

  double prs[RT_ACTION_COUNT];
  prs[RT_DIFFUSE] = cdiffuse.Y();
  prs[RT_SPECULAR] = cspecular.Y();

  for(int i = 1; i < RT_ACTION_COUNT - 1; i++)
    prs[i] += prs[i - 1];

  static_assert(RT_ABSORB == RT_ACTION_COUNT - 1, "");
  prs[RT_ABSORB] = 1;

  if(prs[RT_SPECULAR] > 1.01)
  {
    errs() << "Warning: diffuse plus specular coefficients are greater than 1"
	    << " -- theoreticall, this breaks photons mapping" << endl;
    errs() << cdiffuse << ", " << cspecular << endl;
  }


  const RT_ACTION action = fn(p, incident, acc * cdiffuse, prs);

  if(action == RT_ABSORB)
    return;

  if(action == RT_SPECULAR)
  {
    if((rand() % 101) * 0.01 > r)
    {
      // Refraction.
      // Q: does this need to be multipled by 1) cspecular.Y() and 2) (1 - r)?
      raytrace_russian(p, ray_transmitted, acc * cspecular, fn, depth + 1);
    }
    else
      // Reflection.
      // Q: same as above, except r?
      raytrace_russian(p, ray_reflected, acc * cspecular, fn, depth + 1);
  }
  else
  {
    assert(action == RT_DIFFUSE);
    Vector3D outgoing;
#if 0
    do
    {
      // Cosine distribution.
      const double phi = asin(-1 + rand() * 1. / RAND_MAX);
      const double theta = 2 * M_PI * rand() * 1. / RAND_MAX;

      const Vector3D p1(ray_reflected[1], -ray_reflected[0], ray_reflected[2]);
      const Vector3D p2(ray_reflected.cross(p1));
      const double cosphi = cos(phi), sinphi = sin(phi),
		   costheta = cos(theta), sintheta = sin(theta);
      outgoing = cosphi * ray_reflected + sinphi * costheta * p1 +
					  sinphi * sintheta * p2;
    }
    while(outgoing.dot(normal) <= 0);
#else
    outgoing = ray_reflected;
#endif

    raytrace_russian(p, outgoing, acc * cdiffuse * (1./M_PI), fn, depth + 1);
  }
}
