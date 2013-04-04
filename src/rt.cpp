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

RayTracer::RayTracer(const FlatList &geo)
  : m_geo(geo)
  , m_miss_colour([](const Point3D &, const Vector3D &) { return g_opts->miss_colour; })
{
}

bool RayTracer::raytrace(const Point3D &src, const Vector3D &ray, const RaytraceFn &fn)
{
  const Point3D dst = src + ray;

  HitReporter hr(fn);
  for(auto &geo : m_geo)
  {
    hr.to = hr.primary = &geo;
    if(!geo.prim->intersect(geo.invtrans * src, geo.invtrans * dst, hr))
      return false;
  }

  return true;
}

bool RayTracer::raytrace_within(const Point3D &src, const Vector3D &ray,
				double tlo, double thi)
{
  return !raytrace(src, ray,
    [tlo, thi](const HitInfo &hi)
  {
    return hi.t < tlo || hi.t > thi;
  });
}

bool RayTracer::raytrace_min(const Point3D &src, const Vector3D &ray,
    double tlo, HitInfo &hi)
{
  hi.t = numeric_limits<double>::max();
  raytrace(src, ray, [tlo, &hi] (const HitInfo &_hi)
      {
	if(_hi.t < hi.t && _hi.t >= tlo)
	  hi = _hi;
	return true;
      });
  const bool hit = hi.t < numeric_limits<double>::max();
  if(hit)
  {
    assert(hi.primary);
    hi.normal = hi.primary->trans_normal * hi.normal;
    hi.u = hi.primary->trans_normal * hi.u;
    hi.v = hi.primary->trans_normal * hi.v;
    hi.normal.normalize();
    hi.u.normalize();
    hi.v.normalize();
    hi.primary->mat->get_normal(hi.normal, hi.uv, hi.u, hi.v);
    assert(normalized(hi.normal));
  }
  return hit;
}

double fresnel(double n1, double n2, double cosi, double cost)
{
  double rperp = (n1 * cosi - n2 * cost) / (n1 * cosi + n2 * cost);
  rperp *= rperp;
  double rpar = (n1 * cost - n2 * cosi) / (n1 * cost + n2 * cosi);
  rpar *= rpar;
  return (rpar + rperp) / 2;
}

// Returns the proportion of specular power that should go to reflection. The
// rest should go to refraction.
double compute_specular(const Vector3D &incident,     // Must be unit.
			const FlatGeo *obj,
			const FlatGeo *med,
			const FlatGeo *to,
			const Vector3D &_n,	      // The normal.
			Vector3D &ray_reflected,
			Vector3D &ray_transmitted)
{
  const Material &mmedium = med ? *med->mat : PhongMaterial::air;
  const Material &mobj = obj ? *obj->mat : PhongMaterial::air;
  const Material &mto = to ? *to->mat : PhongMaterial::air;
  const bool penetrating = incident.dot(_n) < 0;
  const Vector3D n = penetrating ? _n : -_n;
  ray_reflected = incident - 2 * incident.dot(n) * n;
  ray_reflected.normalize();

  if(!mto.transmissive())
    // Only reflective. This applies to both sides, by edict.
    return 1;

  const double n1 = penetrating ? mmedium.ri() : mobj.ri();
  const double n2 = penetrating ? mobj.ri() : mmedium.ri();
  const double cosi = incident.dot(-n);
  const double sini = sqrt(1 - cosi * cosi);
  const double sint = n1 / n2 * sini;

  if(sint >= 1)
    // Total internal reflection.
    return 1;

  const double cost = sqrt(1 - sint * sint);
  ray_transmitted = n1/n2 * incident + (n1/n2 * cosi - cost) * n;
  ray_transmitted.normalize();

  if(!mto.reflective())
    return 0;

  return fresnel(n1, n2, cosi, cost);
}

Colour RayTracer::raytrace_recursive(const LightingModel &model,
				     const Point3D &src,
				     const Vector3D &incident,
				     double *dist,
				     double acc,
				     int depth)
{
  const double threshold = 0.02;

  if(dist)
    *dist = -1.;

  assert(normalized(incident));

  HitInfo hi;
  if(!raytrace_min(src, incident, RT_EPSILON, hi))
    return m_miss_colour(src, incident);

  if(dist)
    *dist = hi.t;

  if(depth > 15)
  {
    // This can happen in certain situations (eg. perfect mirrors or
    // repeated total internal reflection on the _inside_ of certain
    // primitives). The best thing we can do in this situations is just
    // calculate the phong lighting.
    //return model.compute_lighting(*this, src, incident, t, *g, mat, normal, uv, u, v, 1);
    return Colour(1);
  }

  const Point3D p = src + hi.t * incident;
  Vector3D ray_reflected, ray_transmitted;
  const double r = compute_specular(incident, hi.primary, hi.from == hi.primary ? hi.to : hi.from, hi.to, hi.normal, ray_reflected, ray_transmitted);

  Colour rv(0);

  Colour cdirect = model.compute_lighting(*this, src, incident, hi, r);
  rv += cdirect;

  const Colour cspecular = hi.tomat().ks(hi.uv);
  const Colour creflected = hi.tomat().reflective() ? r * cspecular : Colour(0);
  const Colour ctransmitted = hi.tomat().transmissive() ? (1 - r) * cspecular : Colour(0);
  const double acc_reflected = acc * creflected.Y();
  const double acc_transmitted = acc * ctransmitted.Y();

  if(acc_reflected > threshold)
  {
    const Colour crec = creflected * raytrace_recursive(model, p, ray_reflected, 0, acc_reflected, depth + 1);
    rv += crec;
  }

  if(acc_transmitted > threshold && r < 0.99)
  {
    double dist_rec;
    const Colour crec = raytrace_recursive(model, p, ray_transmitted, &dist_rec, acc_transmitted, depth + 1);
    Colour cattenuated = cspecular;
    if(dist_rec > -1)
      cattenuated.pow(dist_rec);
    rv += (1 - r) * cattenuated * crec;
  }

  return rv;
}

void RayTracer::raytrace_russian(const Point3D &src,
		      const Vector3D &incident, const Colour &acc,
		      const RussianFn &fn, int depth)
{
  assert(normalized(incident));

  HitInfo hi;
  if(!raytrace_min(src, incident, RT_EPSILON, hi))
    return;

  if(depth > 12)
    // We've no use for information about photons hitting spec surfaces, so...
    return;

  Vector3D ray_reflected;
  Vector3D ray_transmitted;
  const double r = compute_specular(incident, hi.primary, hi.primary == hi.from ? hi.to : hi.from, hi.to, hi.normal,
				    ray_reflected, ray_transmitted);

  const Point3D p = src + hi.t * incident;
  // Giving incorrect uv coordinates to a medium won't hurt...
  const Colour cdiffuse = hi.tomat().kd(hi.uv);
  Colour cspecular = hi.tomat().ks(hi.uv);

  double prs[RT_ACTION_COUNT];
  prs[RT_DIFFUSE] = cdiffuse.Y();
  prs[RT_SPECULAR] = cspecular.Y();

  if(!hi.tomat().reflective() && !hi.tomat().transmissive())
    cspecular = Colour(0);

  for(int i = 1; i < RT_ACTION_COUNT - 1; i++)
    prs[i] += prs[i - 1];

  static_assert(RT_ABSORB == RT_ACTION_COUNT - 1, "");
  prs[RT_ABSORB] = 1;

  if(prs[RT_SPECULAR] > 1.01)
  {
    errs() << "Warning: diffuse plus specular coefficients are greater than 1"
	    << " -- theoretically, this breaks photons mapping" << endl;
    errs() << cdiffuse << ", " << cspecular << endl;
  }

  const RT_ACTION action = fn(p, ray_reflected, acc * cdiffuse, prs);

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

    // Things might look better if, instead of just using the reflected
    // direction, I distributed diffusely reflected rays according to a cosine
    // distribution contered on the reflective direction. But that's difficult.
    outgoing = ray_reflected;

    raytrace_russian(p, outgoing, acc * cdiffuse, fn, depth + 1);
  }
}
