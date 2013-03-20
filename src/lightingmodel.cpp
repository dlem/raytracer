#include "lightingmodel.hpp"
#include "material.hpp"
#include "rt.hpp"
#include "cmdopts.hpp"

Colour PhongModel::compute_lighting(RayTracer &rt,
				  const Point3D &src,
				  const Vector3D &ray,
				  const double t,
				  const FlatGeo &geo,
				  const Vector3D &normal,
				  const Point2D &uv,
				  const Vector3D &u,
				  const Vector3D &v,
				  const double refl_attn
				  ) const
{
  const Point3D phong_P = src + t * ray;

  // Projection drawing stuff.
  const Colour projc(1, 0, 0);
  const bool use_proj = GETOPT(draw_caustic_pm) &&
			m_caustics.test_pm(rt, phong_P, normal);
  const Colour phong_kd = use_proj ? projc * geo.mat->kd(uv).Y() : geo.mat->kd(uv);

  Vector3D phong_n = normal;
  geo.mat->get_normal(phong_n, uv, u, v);
  const Colour &phong_ks = refl_attn * geo.mat->ks(uv);
  const double phong_p = geo.mat->shininess(uv);
  Vector3D phong_v = src - phong_P;
  phong_v.normalize();

  const Colour ambient = m_gimap ?
    m_gimap->query_radiance(phong_P, -ray) : m_ambient;

  Colour rv = ambient * phong_kd;

  for(auto &light : m_lights)
  {
    Vector3D phong_ell = light->position - phong_P;
    const double dist = phong_ell.normalize();

    const bool shadow = phong_n.dot(phong_ell) < -0.01;

    if(rt.raytrace_within(phong_P, phong_ell, 0.001, dist))
      // Skip this light. There's an obstacle => shadow.
      continue;

    // This is the diffuse term.
    Colour phong_ro = phong_kd;

    // Add the specular term, which is based on how close we are to the light's
    // reflection angle.
    Vector3D phong_r = 2 * phong_ell.dot(phong_n) * phong_n - phong_ell;
    // We can maybe avoid this normalize?
    phong_r.normalize();
    const double refl = phong_r.dot(phong_v);

    if(refl > 0)
      phong_ro = phong_ro + phong_ks * (pow(refl, phong_p) * phong_n.dot(phong_ell));

    // Compute the attenuation.
    const double *falloff = light->falloff;
    const double c0 = falloff[0], c1 = falloff[1], c2 = falloff[2];
    const double attenuation = c0 + c1 * dist + c2 * dist * dist;
    
    // Light mostly propagates towards the normal -- adjust the outgoing light
    // based on normal vs view vector.
    const Colour terms = phong_ro * (phong_ell.dot(phong_n) / attenuation);

    rv += light->colour * terms;
  }

  if(m_caustics.size() > 50)
  {
    Colour c = m_caustics.query_radiance(phong_P, -ray);
    rv += c;
  }

  return rv;
}

Colour PhotonDrawModel::compute_lighting(RayTracer &rt,
			      const Point3D &src,
			      const Vector3D &ray,
			      const double t,
			      const FlatGeo &geo,
			      const Vector3D &normal,
			      const Point2D &uv,
			      const Vector3D &u,
			      const Vector3D &v,
			      const double refl_attn
			      ) const
{
#if 0
  const Point3D p = src + t * ray;
  Vector3D pos_rel;
  m_map.query_photon(p, pos_rel);

  if(pos_rel.length() <= 10)
    return Colour(1, 1, 1);
  else
    return Colour(0, 0, 0);
#endif
  const Point3D p = src + t * ray;
  Vector3D pos_rel;
  Colour c = m_map.query_photon(p, pos_rel);

  if(pos_rel.length() <= 10)
    return c;
  else
    return Colour(0, 0, 0);

}

Colour PhotonsOnlyModel::compute_lighting(RayTracer &rt,
			      const Point3D &src,
			      const Vector3D &ray,
			      const double t,
			      const FlatGeo &geo,
			      const Vector3D &normal,
			      const Point2D &uv,
			      const Vector3D &u,
			      const Vector3D &v,
			      const double refl_attn
			      ) const
{
  const Point3D p = src + t * ray;
  const Vector3D outgoing = -ray;
  return m_map.query_radiance(p, outgoing);
}
