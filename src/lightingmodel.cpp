#include "lightingmodel.hpp"
#include "material.hpp"
#include "rt.hpp"
#include "cmdopts.hpp"

static double occlusion(RayTracer &rt, Light *light, const Point3D &pt)
{
  const double epsilon = 0.001;
  const int resolution = GETOPT(shadow_grid);
  const double radius = light->radius;

  Vector3D w = light->position - pt;
  const double dist = w.normalize();

  if(radius <= 0 || !GETOPT(soft_shadows) || resolution <= 1)
  {
    if(rt.raytrace_within(pt, w, epsilon, dist))
      return 0;
    return 1;
  }

  const double maxd = radius / dist;
  double occ = 0;
  int count = 0;
  Vector3D u(w[1], -w[0], 0);
  u.normalize();
  const Vector3D v(w.cross(u));
  for(int i = 0; i < resolution; i++)
  {
    const double du = maxd * (-1 + (i + 0.5) * 2. / resolution);
    for(int j = 0; j < resolution; j++)
    {
      const double dv = maxd * (-1 + (j + 0.5) * 2. / resolution);

      const double sumsqr = du * du + dv * dv;
      if(sumsqr < 1)
      {
	const double dw = sqrt(1 - sumsqr);
	const Vector3D ray = du * u + dv * v + dw * w;
	if(!rt.raytrace_within(pt, ray, epsilon, dist))
	  occ += 1;
	count += 1;
      }
    }
  }
  return occ / count;
}

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

    const double occ = occlusion(rt, light, phong_P);
    if(occ < 0.01)
      continue;

    if(phong_ell.dot(phong_n) <= 0)
      // This can happen even with +if occlusion due to area lights.
      // It'll cause a darkening artifact if we continue.
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

    rv += light->colour * occ * terms;
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
